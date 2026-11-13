#pragma once

#include <brpc/channel.h>
#include <google/protobuf/util/json_util.h>

#include "dao/transmite_dao.h"
#include "amqp/mq_client.h"
#include "transmite.pb.h"
#include "jwt_util.h"
#include "random_id.h"
#include "json/json_codec.h"
#include "user.pb.h"
#include "etcd/discover/service_discovery.h"

class TransmiteManager{
public:
    using Ptr = std::shared_ptr<TransmiteManager>;  

    TransmiteManager(TransmiteDao::Ptr dao, 
        const std::string& exchange, const std::string& binding_key, MqClient::Ptr mq_client);

    ::linkchat::GetTransmitTargetRsp GetTransmitTarget(const ::linkchat::NewMessageReq& req);

    ~TransmiteManager();
private:
    TransmiteDao::Ptr dao_;

    std::string exchange_;
    std::string binding_key_;
    MqClient::Ptr mq_client_;
};