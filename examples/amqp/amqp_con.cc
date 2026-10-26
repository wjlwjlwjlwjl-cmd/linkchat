#include <amqpcpp.h>
#include <amqpcpp/libev.h>

int main(){
    const std::string broker_url = "amqp://admin:123456@rabbitmq-service:5672/"; // docker 网络自动解析ip
    const std::string exchange_name = "test_exchange";
    const std::string queue_name = "test_queue";
    const std::string routing_key = "test_routing_key";

    auto* ev_loop = EV_DEFAULT; //初始化网络通信循环句柄
    AMQP::LibEvHandler handler(ev_loop); //AMQP 网络通信句柄
    AMQP::TcpConnection connection(&handler, AMQP::Address(broker_url));
    AMQP::TcpChannel channel(&connection);

    //这里采取更经典的方式：生产者负责完成交换机、队列、路由键的绑定
    //AMQP-CPP 没有 Spring AMQP manual 那种正常执行自动 ack 的选项，只有手动和自动
    channel.consume(queue_name)
    .onMessage([&](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered){
        std::string msg = std::string(message.body(), message.bodySize());
        std::cout << "消费者消费成功：" << msg << std::endl;;
        channel.ack(deliveryTag);
    })
    .onError([&](const char *message){
        std::cout << "消费消息失败: " << message << std::endl;
    });

    ev_run(ev_loop); //开启网络事件循环
    return 0;
}