#include <amqpcpp.h>
#include <amqpcpp/libev.h>

int main() {
    const std::string broker_url = "amqp://admin:123456@rabbitmq-service:5672/"; // docker 网络自动解析ip
    const std::string exchange_name = "test_exchange";
    const std::string queue_name = "test_queue";
    const std::string routing_key = "test_routing_key";

    //TcpConnenction 是 AMQP-CPP 通用的连接对象，IO 方式取决于传入的句柄
    auto* ev_loop = EV_DEFAULT; //初始化网络通信循环句柄
    AMQP::LibEvHandler handler(ev_loop); //AMQP 网络通信句柄
    AMQP::TcpConnection connection(&handler, AMQP::Address(broker_url));
    AMQP::TcpChannel channel(&connection);

    channel.declareExchange(exchange_name, AMQP::ExchangeType::direct)
    .onSuccess([&]() {
        std::cout << "Exchange " << exchange_name << " declared successfully" << std::endl;

        // 如果需要绑定死信队列：
        // AMQP::table args
        // x-dead-letter-exchange，DLX，死信交换机
        // x-dead-letter-queue，DLQ，死信队列
        // x-message-ttl，过期时间
        channel.declareQueue(queue_name)
        .onSuccess([&](const std::string& name, uint32_t message_count, uint32_t consumer_count) {
            std::cout << "Queue " << queue_name << " declared successfully" << std::endl;
            std::cout << "messagecount: " << message_count << ", " << "consumercount: " << consumer_count << std::endl;

            channel.bindQueue(exchange_name, queue_name, routing_key)
            .onSuccess([&]() {
                std::cout << "Bound queue " << queue_name << " to exchange " << exchange_name << ", routing_key: " << routing_key << std::endl;
                bool ret = channel.publish(exchange_name, routing_key, "this is a message send to brocker from publisher");
                if (!ret) {
                    std::cout << "Failed to publish message" << std::endl;
                }
                else {
                    std::cout << "Message published successfully" << std::endl;
                }
            })
            .onError([&](const char* message) {
                std::cout << "Failed to bind queue: " << message << std::endl;
                abort();
            });
        })
        .onError([&](const char* message) {
            std::cout << "Failed to declare queue: " << message << std::endl;
            abort();
        });
    })
    .onError([&](const char* message) {
        std::cout << "Failed to declare exchange: " << message << std::endl;
        abort();
    });
    ev_run(ev_loop); //开启网络事件循环
    return 0;
}
