#pragma once

#include <brpc/channel.h>

#include "etcd/discover/service_discovery.h"
#include "dao/transmite_dao.h"
#include "amqp/mq_client.h"
#include "domain/transmite_dto.h"

class TransmiteManager{
public:
    using Ptr = std::shared_ptr<TransmiteManager>;  

    TransmiteManager(TransmiteDao::Ptr dao, 
        const std::string& exchange, const std::string& binding_key, MqClient::Ptr mq_client,
        SvcDiscovery::Ptr svc_discovery, const std::string& etcd_server_ip,
        const std::string& protocol, int connect_timeout_ms, int timeout_ms, int max_retry);

    GetTransmitTargetDTO GetTransmitTarget(const NewMessageDTO& req);

    ~TransmiteManager();
private:
    TransmiteDao::Ptr dao_;

    std::string exchange_;
    std::string binding_key_;
    MqClient::Ptr mq_client_;

    SvcDiscovery::Ptr svc_discovery_;
    ::brpc::ChannelOptions options_;

    std::string etcd_server_ip_;
    std::string protocol_;
    int connect_timeout_ms_;
    int timeout_ms_;
    int max_retry_;

    // 获取会话的所有成员 id，失败时返回空列表
    std::vector<std::string> GetChatSessionMembers_(const std::string& user_id,
        const std::string& chat_session_id);
};