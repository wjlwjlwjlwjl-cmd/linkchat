//amqp 操作封装，声明交换机、队列、绑定关系等；发布，订阅；启动网络时间循环


#pragma once

#include <string>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>

#include "constants/amqp_constants.hpp"

/// @brief AMQP 参数
struct DeclareSettings{
    DeclareSettings() = delete;

    DeclareSettings(const std::string& exchange, const std::string& exchange_type,  const std::string& queue, const std::string& binding_key, int ttl = 0);

    std::string _exchange;
    std::string _exchange_type;
    std::string _queue;
    std::string _binding_key;
    int _ttl; //过期时间

    //死信队列的会组装，其余的自行指定
    std::string get_dl_exchange();
    std::string get_dl_queue();
    std::string get_dl_binding_key();
};

DeclareSettings::DeclareSettings(const std::string& exchange, const std::string& exchange_type, 
    const std::string& queue, const std::string& binding_key, int ttl)
    : _exchange(exchange)
    , _exchange_type(exchange_type)
    , _queue(queue)
    , _binding_key(binding_key)
    , _ttl(ttl)
{}

std::string DeclareSettings::get_dl_exchange(){
    return DL::DLX_PREFIX + _exchange;
}

std::string DeclareSettings::get_dl_queue(){
    return DL::DLQ_PREFIX + _queue;
}

std::string DeclareSettings::get_dl_binding_key(){
    return DL::DLX_BINDING_KEY + _queue;
}


/// @brief AMQP Broker 操作封装
class MQClient{
public:
    //libev 网络库底层并不是线程安全的，因此对于 _channel 的任何操作，都需要加锁处理，否则会导致行为混乱
    //同时，队列、交换机、路由关系的回调也都是异步的，所以要等待全部完成后，外层函数再返回。这里采取值捕获和条件变量的方式
    using MessageCallback = std::function<void(const char* message, int message_size)>;

    /// @brief 初始化网络协议等
    /// @param broker_url AMQP 服务器连接
    MQClient(const std::string& broker_url);

    ~MQClient();

    /// @brief 声明交换机、队列、绑定路由键，如果需要的话，绑定死信交换机、死信队列
    /// @param 信息
    void Declare(DeclareSettings);

    /// @brief 发布消息
    /// @param 交换机
    /// @param 路由建
    /// @param 消息
    bool Publish(const std::string&, const std::string&, const char*);

    /// @brief 消费消息
    /// @param 队列 
    /// @param 消息回调 
    void Consume(const std::string&, MessageCallback&);
private:
    /// @brief 单纯负责声明交换机、队列、路由键等
    /// @param  信息
    void _Declare(DeclareSettings&, AMQP::Table&); 

    static void Callback(struct ev_loop *loop, ev_async *watcher, int revents);

    std::mutex _declare_mtx; //用于互斥（libev 底层非线程安全）
    std::mutex _mtx; //用于条件控制
    std::condition_variable _cv;
    struct ev_loop* _ev_loop; //网络事件循环
    ev_async _ev_async;
    AMQP::LibEvHandler _handler;
    AMQP::TcpConnection _connection;
    AMQP::TcpChannel _channel;
    std::thread _async_thread; //启动一个异步线程完成网络事件循环
};

MQClient::MQClient(const std::string& broker_url)
    : _ev_loop(EV_DEFAULT)
    , _handler(_ev_loop)
    , _connection(&_handler, AMQP::Address(broker_url))
    , _channel(&_connection)
    , _async_thread(std::thread([this](){ev_run(_ev_loop);})) //在异步线程中开启网络时间循环
{}

MQClient::~MQClient(){
    //在析构中，要结束网络事件循环，需要异步进行，否则在网络事件循环中等待的线程可能会出现未定义行为，
    //这样可以安全的通知等待的线程网络事件循环结束并执行相应回调；
    //并且完成线程的join

    ev_async_init(&_ev_async, MQClient::Callback); //初始化异步通知机制
    ev_async_start(_ev_loop, &_ev_async); //启动异步通知机制
    ev_async_send(_ev_loop, &_ev_async); //libev 本身不是线程安全的，所以通过异步通知机制，让其他等待事件的线程“醒来”后退出
    _async_thread.join();
}

void MQClient::Callback(struct ev_loop *loop, ev_async *watcher, int revents){
    ev_break(loop, EVBREAK_ALL);
}

void MQClient::_Declare(DeclareSettings& settings, AMQP::Table& tb){
    std::unique_lock<std::mutex> declare_lock(_declare_mtx); //维持互斥
    std::unique_lock<std::mutex> lock(_mtx); //条件关系

    //因为异步关系，这里需要值捕获，否则异步会导致变量失效
    //C++17，= 会捕获 this，但是 C++20 后不会，因此这里按需捕获
    _channel.declareExchange(settings._exchange, EXCHANGE_TYPE::getExchangeType(settings._exchange_type))
    .onSuccess([=, this](){
        _channel.declareQueue(settings._queue, tb)
        .onSuccess([=, this](const std::string& name, uint32_t messagecount, uint32_t consumercount){
            _channel.bindQueue(settings._exchange, settings._queue, settings._binding_key)
            .onSuccess([=, this](){
                INF("Exchange: {}, Queue: {}, Binding Key: {}, init successfully", settings._exchange, settings._queue, settings._binding_key);
                _cv.notify_all();
            })
            .onError([=, this](const char* message){
                ERR("Binding {} <=> {} error, key: {}, reason: {}", settings._exchange, settings._queue, settings._binding_key, message);
                _cv.notify_all();
                abort();
            });
        })
        .onError([=, this](const char* message){
            ERR("Queue {} declared error: {}", settings._queue, message);
            _cv.notify_all();
            abort();
        });
    })
    .onError([=, this](const char* message){
        INF("Exchange {} declared error: {}", settings._exchange, message);
        _cv.notify_all();
        abort();
    });

    //等待异步声明完成
    _cv.wait(lock);
}


void MQClient::Declare(DeclareSettings settings){
    AMQP::Table tb;

    if(settings._ttl > 0){
        //延时队列，先声明死信队列
        DeclareSettings dl_settings(settings.get_dl_exchange(), EXCHANGE_TYPE::DIRECT, settings.get_dl_queue(), 
            settings.get_dl_binding_key());
        _Declare(dl_settings, tb);

        tb[DL::DLX_OP] = settings.get_dl_exchange();
        tb[DL::DLQ_OP] = settings.get_dl_binding_key();
        tb[DL::MSG_TTL] = settings._ttl * 1000;
    }

    _Declare(settings, tb);
}

bool MQClient::Publish(const std::string& exchange, const std::string& routing_key, const char* msg){
    std::unique_lock<std::mutex> declare_lock(_declare_mtx);
    return _channel.publish(exchange, routing_key, msg);
}

void MQClient::Consume(const std::string& queue, MessageCallback& cb){
    std::unique_lock<std::mutex> declare_lock(_declare_mtx);
    std::unique_lock<std::mutex> lock(_mtx);

    _channel.consume(queue)
    .onMessage([=, this](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
        cb(message.body(), message.bodySize());
        _channel.ack(deliveryTag);
    })
    .onError([=](const char* message){
        ERR("Failed to consume from {}: {}", queue, message);
        abort();
    })
    .onSuccess([=, this](){
        INF("Started consuming from {}", queue);
        _cv.notify_all();
    });

    _cv.wait(lock);
}