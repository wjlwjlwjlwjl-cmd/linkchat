#include <amqpcpp.h>
#include <amqpcpp/libev.h>

#include "mq_client.hpp"

class Consumer{
public:
    using ptr = std::shared_ptr<MQClient>;

    Consumer() = delete;
    Consumer(Consumer::ptr& mq_client, DeclareSettings& settings);

    void Consume(MQClient::MessageCallback&&);
private:
    Consumer::ptr _mq_client;
    DeclareSettings _settings;
    MQClient::MessageCallback _cb;
};

Consumer::Consumer(Consumer::ptr& mq_client, DeclareSettings& settings)
    : _mq_client(mq_client)
    , _settings(settings)
{
    _mq_client->Declare(_settings); //按照经典的方式，一般是生产者负责声明队列信息，但是这里采取双方都声明的方式保证安全性
}

void Consumer::Consume(MQClient::MessageCallback&& cb){
    _cb = std::move(cb);
    if(_settings._ttl > 0){
        //延时队列，订阅死信队列
        _mq_client->Consume(_settings.get_dl_queue(), _cb);
    }
    else{
        //订阅普通队列即可
        _mq_client->Consume(_settings._queue, _cb);
    }
}