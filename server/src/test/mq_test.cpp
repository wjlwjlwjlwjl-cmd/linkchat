#include <chrono>

#include "../adapter/amqp/mq_client.hpp"
#include "../constants/amqp_constants.hpp"

using namespace std::chrono_literals;

MQClient mq_client("amqp://admin:123456@rabbitmq-service:5672");

void MQ_DECLARE_NO_TTL_TEST(){
    DeclareSettings settings("test_exchange", EXCHANGE_TYPE::DIRECT, "test_queue", "test_binding_key");
    mq_client.Declare(settings);
    
    DeclareSettings settings1("test_exchange1", EXCHANGE_TYPE::FANOUT, "test_queue1", "test_binding_key1");
    mq_client.Declare(settings1);

    DeclareSettings settings2("test_exchange2", EXCHANGE_TYPE::TOPIC, "test_queue2", "test_binding_key2");
    mq_client.Declare(settings2);
}

void MQ_DECLARE_TTL_TEST(){
    DeclareSettings settings("test_exchange3", EXCHANGE_TYPE::DIRECT, "test_queue3", "test_binding_key3", 9);
    mq_client.Declare(settings);
    
    DeclareSettings settings1("test_exchange4", EXCHANGE_TYPE::FANOUT, "test_queue4", "test_binding_key4", 9);
    mq_client.Declare(settings1);

    DeclareSettings settings2("test_exchange5", EXCHANGE_TYPE::TOPIC, "test_queue5", "test_binding_key5", 9);
    mq_client.Declare(settings2);
}

void PUBLISH_TEST(){
    mq_client.Publish("test_exchange3", "test_binding_key3", "this is a test msg first");
    std::this_thread::sleep_for(1s);
    mq_client.Publish("test_exchange3", "test_binding_key3", "this is a test msg second");
    std::this_thread::sleep_for(1s);
    mq_client.Publish("test_exchange3", "test_binding_key3", "this is a test msg third");
}

void CONSUME_TEST(){
    auto cb = [](const char* msg, int msgSize){
        std::cout << "Consumer get message: " << std::string(msg, msgSize) << std::endl;
    };
    mq_client.Consume("test_queue3", cb);
}

int main(){
    init_logger();
    std::thread con_thread([=](){CONSUME_TEST();});
    getchar();
    con_thread.join();
    return 0;
}