#include <amqpcpp.h>
#include <amqpcpp/libev.h>

#include "mq_client.hpp"

class Publisher{
public:
    using ptr = std::shared_ptr<MQClient>;

    Publisher() = delete;
    Publisher(const Publisher::ptr& mq_client, const DeclareSettings& settings);

    bool Publish(const char* msg);
private:
    Publisher::ptr _mq_client;
    DeclareSettings _settings;
};

Publisher::Publisher(const Publisher::ptr& mq_client, const DeclareSettings& settings)
    : _mq_client(mq_client)
    , _settings(settings)
{
    _mq_client->Declare(_settings);
}

bool Publisher::Publish(const char* msg){
    return _mq_client->Publish(_settings._exchange, _settings._binding_key, msg);
}