#pragma once

#include <brpc/channel.h>

#include "dao/transmite_dao.h"
#include "amqp/mq_client.h"
#include "domain/transmite_dto.h"

class TransmiteManager{
public:
    using Ptr = std::shared_ptr<TransmiteManager>;  

    TransmiteManager(TransmiteDao::Ptr dao, 
        const std::string& exchange, const std::string& binding_key, MqClient::Ptr mq_client);

    GetTransmitTargetDTO GetTransmitTarget(const NewMessageDTO& req);

    ~TransmiteManager();
private:
    TransmiteDao::Ptr dao_;

    std::string exchange_;
    std::string binding_key_;
    MqClient::Ptr mq_client_;
};