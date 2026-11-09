#include <amqpcpp.h>
#include <amqpcpp/libev.h>

#include "mq_client.h"

class Consumer {
public:
    using Ptr = std::shared_ptr<MqClient>;

    Consumer() = delete;
    inline Consumer(Ptr& mq_client, DeclareSettings& settings);

    inline void Consume(MqClient::MessageCallback&& callback);
private:
    Ptr mq_client_;
    DeclareSettings settings_;
    MqClient::MessageCallback callback_;
};

inline Consumer::Consumer(Ptr& mq_client, DeclareSettings& settings)
    : mq_client_(mq_client)
    , settings_(settings)
{
    mq_client_->Declare(settings_); //按照经典的方式，一般是生产者负责声明队列信息，但是这里采取双方都声明的方式保证安全性
}

inline void Consumer::Consume(MqClient::MessageCallback&& callback) {
    callback_ = std::move(callback);
    if (settings_.ttl > 0) {
        //延时队列，订阅死信队列
        mq_client_->Consume(settings_.GetDlQueue(), callback_);
    }
    else {
        //订阅普通队列即可
        mq_client_->Consume(settings_.queue, callback_);
    }
}
