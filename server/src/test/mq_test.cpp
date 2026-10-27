#include <chrono>

#include "../adapter/amqp/mq_client.hpp"
#include "../adapter/amqp/publisher.hpp"
#include "../adapter/amqp/consumer.hpp"
#include "../adapter/redis/redis_factory.hpp"
#include "../constants/amqp_constants.hpp"

using namespace std::chrono_literals;

auto mq_client = std::make_shared<MQClient>("amqp://admin:123456@rabbitmq-service:5672");

void PUBLISH_TEST(){
    DeclareSettings settings("test_exchange3", EXCHANGE_TYPE::DIRECT, "test_queue3", "test_binding_key3", 9);
    Publisher publisher(mq_client, settings);
    publisher.Publish("this is a message test publishing");
}

void CONSUME_TEST(){
    //延时队列
    DeclareSettings settings("test_exchange3", EXCHANGE_TYPE::DIRECT, "test_queue3", "test_binding_key3", 9);
    Consumer consumer(mq_client, settings);
    consumer.Consume([](const char* msg, int size){
        std::cout << std::string(msg, size) << std::endl;
    });
    getchar();
}

int main(){
    init_logger();
    std::thread con_thread([=](){CONSUME_TEST();});
    std::this_thread::sleep_for(2s);
    std::thread pub_thread([=](){PUBLISH_TEST();});
    con_thread.join();
    pub_thread.join();
    return 0;
}