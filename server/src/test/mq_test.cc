#include <chrono>

#include "../adapter/amqp/mq_client.h"
#include "../adapter/amqp/publisher.h"
#include "../adapter/amqp/consumer.h"
#include "../adapter/redis/redis_factory.h"
#include "../constants/amqp_constants.h"

using namespace std::chrono_literals;

auto g_mq_client = std::make_shared<MqClient>("amqp://admin:123456@rabbitmq-service:5672");

void PublishTest() {
    DeclareSettings settings{"test_exchange3", exchange_type::kDirect, "test_queue3", "test_binding_key3", 9};
    Publisher publisher(g_mq_client, settings);
    publisher.Publish("this is a message test publishing");
}

void ConsumeTest() {
    //延时队列
    DeclareSettings settings{"test_exchange3", exchange_type::kDirect, "test_queue3", "test_binding_key3", 9};
    Consumer consumer(g_mq_client, settings);
    consumer.Consume([](const char* msg, int size) {
        std::cout << std::string(msg, size) << std::endl;
    });
    getchar();
}

int main() {
    InitLogger();
    std::thread consume_thread([=]() { ConsumeTest(); });
    std::this_thread::sleep_for(2s);
    std::thread publish_thread([=]() { PublishTest(); });
    consume_thread.join();
    publish_thread.join();
    return 0;
}
