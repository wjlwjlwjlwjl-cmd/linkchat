#include <amqpcpp.h>
#include <amqpcpp/libev.h>

#include "mq_client.h"

class Publisher {
public:
    using Ptr = std::shared_ptr<MqClient>;

    Publisher() = delete;
    inline Publisher(const Ptr& mq_client, const DeclareSettings& settings);

    inline bool Publish(const char* msg);
private:
    Ptr mq_client_;
    DeclareSettings settings_;
};

inline Publisher::Publisher(const Ptr& mq_client, const DeclareSettings& settings)
    : mq_client_(mq_client)
    , settings_(settings)
{
    mq_client_->Declare(settings_);
}

inline bool Publisher::Publish(const char* msg) {
    return mq_client_->Publish(settings_.exchange, settings_.binding_key, msg);
}
