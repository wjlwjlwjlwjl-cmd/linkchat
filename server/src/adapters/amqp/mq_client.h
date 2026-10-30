//amqp 操作封装，声明交换机、队列、绑定关系等；发布，订阅；启动网络时间循环


#pragma once

#include <string>
#include <amqpcpp.h>
#include <amqpcpp/libev.h>

#include "constants/amqp_constants.h"

/// @brief AMQP 参数
/// 纯数据载体，使用聚合初始化：DeclareSettings{exchange, exchange_type, queue, binding_key, ttl}
struct DeclareSettings {
    std::string exchange;
    std::string exchange_type;
    std::string queue;
    std::string binding_key;
    int ttl = 0; //过期时间

    //死信队列的会组装，其余的自行指定
    inline std::string GetDlExchange() const;
    inline std::string GetDlQueue() const;
    inline std::string GetDlBindingKey() const;
};

inline std::string DeclareSettings::GetDlExchange() const {
    return dead_letter::kDlxPrefix + exchange;
}

inline std::string DeclareSettings::GetDlQueue() const {
    return dead_letter::kDlqPrefix + queue;
}

inline std::string DeclareSettings::GetDlBindingKey() const {
    return dead_letter::kDlxBindingKey + queue;
}


/// @brief AMQP Broker 操作封装
class MqClient {
public:
    //libev 网络库底层并不是线程安全的，因此对于 channel_ 的任何操作，都需要加锁处理，否则会导致行为混乱
    //同时，队列、交换机、路由关系的回调也都是异步的，所以要等待全部完成后，外层函数再返回。这里采取值捕获和条件变量的方式
    using MessageCallback = std::function<void(const char* message, int message_size)>;

    /// @brief 初始化网络协议等
    /// @param broker_url AMQP 服务器连接
    inline MqClient(const std::string& broker_url);

    inline ~MqClient();

    /// @brief 声明交换机、队列、绑定路由键，如果需要的话，绑定死信交换机、死信队列
    /// @param 信息
    inline void Declare(DeclareSettings settings);

    /// @brief 发布消息
    /// @param 交换机
    /// @param 路由建
    /// @param 消息
    inline bool Publish(const std::string& exchange, const std::string& routing_key, const char* msg);

    /// @brief 消费消息
    /// @param 队列
    /// @param 消息回调
    inline void Consume(const std::string& queue, MessageCallback& callback);
private:
    /// @brief 单纯负责声明交换机、队列、路由键等
    /// @param  信息
    inline void DeclareImpl(DeclareSettings& settings, AMQP::Table& table);

    static void Callback(struct ev_loop* loop, ev_async* watcher, int revents);

    std::mutex declare_mutex_; //用于互斥（libev 底层非线程安全）
    std::mutex mutex_; //用于条件控制
    std::condition_variable cv_;
    struct ev_loop* ev_loop_; //网络事件循环
    ev_async ev_async_;
    AMQP::LibEvHandler handler_;
    AMQP::TcpConnection connection_;
    AMQP::TcpChannel channel_;
    std::thread async_thread_; //启动一个异步线程完成网络事件循环
};

inline MqClient::MqClient(const std::string& broker_url)
    : ev_loop_(EV_DEFAULT)
    , handler_(ev_loop_)
    , connection_(&handler_, AMQP::Address(broker_url))
    , channel_(&connection_)
    , async_thread_(std::thread([this]() { ev_run(ev_loop_); })) //在异步线程中开启网络时间循环
{}

inline MqClient::~MqClient() {
    //在析构中，要结束网络事件循环，需要异步进行，否则在网络事件循环中等待的线程可能会出现未定义行为，
    //这样可以安全的通知等待的线程网络事件循环结束并执行相应回调；
    //并且完成线程的join

    ev_async_init(&ev_async_, MqClient::Callback); //初始化异步通知机制
    ev_async_start(ev_loop_, &ev_async_); //启动异步通知机制
    ev_async_send(ev_loop_, &ev_async_); //libev 本身不是线程安全的，所以通过异步通知机制，让其他等待事件的线程“醒来”后退出
    async_thread_.join();
}

inline void MqClient::Callback(struct ev_loop* loop, ev_async* watcher, int revents) {
    ev_break(loop, EVBREAK_ALL);
}

inline void MqClient::DeclareImpl(DeclareSettings& settings, AMQP::Table& table) {
    std::unique_lock<std::mutex> declare_lock(declare_mutex_); //维持互斥
    std::unique_lock<std::mutex> lock(mutex_); //条件关系

    //因为异步关系，这里需要值捕获，否则异步会导致变量失效
    //C++17，= 会捕获 this，但是 C++20 后不会，因此这里按需捕获
    channel_.declareExchange(settings.exchange, exchange_type::GetExchangeType(settings.exchange_type))
    .onSuccess([=, this]() {
        channel_.declareQueue(settings.queue, table)
        .onSuccess([=, this](const std::string& name, uint32_t message_count, uint32_t consumer_count) {
            channel_.bindQueue(settings.exchange, settings.queue, settings.binding_key)
            .onSuccess([=, this]() {
                LOG_INFO("Exchange: {}, Queue: {}, Binding Key: {}, init successfully", settings.exchange, settings.queue, settings.binding_key);
                cv_.notify_all();
            })
            .onError([=, this](const char* message) {
                LOG_ERROR("Binding {} <=> {} error, key: {}, reason: {}", settings.exchange, settings.queue, settings.binding_key, message);
                cv_.notify_all();
                abort();
            });
        })
        .onError([=, this](const char* message) {
            LOG_ERROR("Queue {} declared error: {}", settings.queue, message);
            cv_.notify_all();
            abort();
        });
    })
    .onError([=, this](const char* message) {
        LOG_INFO("Exchange {} declared error: {}", settings.exchange, message);
        cv_.notify_all();
        abort();
    });

    //等待异步声明完成
    cv_.wait(lock);
}


inline void MqClient::Declare(DeclareSettings settings) {
    AMQP::Table table;

    if (settings.ttl > 0) {
        //延时队列，先声明死信队列
        DeclareSettings dl_settings{settings.GetDlExchange(), exchange_type::kDirect, settings.GetDlQueue(),
            settings.GetDlBindingKey()};
        DeclareImpl(dl_settings, table);

        table[dead_letter::kDlxOp] = settings.GetDlExchange();
        table[dead_letter::kDlqOp] = settings.GetDlBindingKey();
        table[dead_letter::kMsgTtl] = settings.ttl * 1000;
    }

    DeclareImpl(settings, table);
}

inline bool MqClient::Publish(const std::string& exchange, const std::string& routing_key, const char* msg) {
    std::unique_lock<std::mutex> declare_lock(declare_mutex_);
    return channel_.publish(exchange, routing_key, msg);
}

inline void MqClient::Consume(const std::string& queue, MessageCallback& callback) {
    std::unique_lock<std::mutex> declare_lock(declare_mutex_);
    std::unique_lock<std::mutex> lock(mutex_);

    channel_.consume(queue)
    .onMessage([=, this](const AMQP::Message& message, uint64_t delivery_tag, bool redelivered) {
        callback(message.body(), message.bodySize());
        channel_.ack(delivery_tag);
    })
    .onError([=](const char* message) {
        LOG_ERROR("Failed to consume from {}: {}", queue, message);
        abort();
    })
    .onSuccess([=, this]() {
        LOG_INFO("Started consuming from {}", queue);
        cv_.notify_all();
    });

    cv_.wait(lock);
}
