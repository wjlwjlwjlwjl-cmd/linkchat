#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "dao/message_dao.h"
#include "etcd/discover/service_discovery.h"
#include "service_constants.h"

class MessageManager{
public:
    using Ptr = std::shared_ptr<MessageManager>;
    MessageManager(
        const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
        int timeout_ms, int max_retry,
        const char* user, const char* passwd, const char* db_name = "linkchat", const char* host = "mysql-service",
        unsigned int port = 3306, int connection_pool_size = 5,
        const char* elastic_url = "http://elastic:123456@elasticsearch-service:9200/",
        const char* index_name = "message", const char* doc_type = "_doc", const char* id = "");

    //消息持久化
    bool PersistMessage(const MessageInfo& info);

    // 三个 brpc 接口
    std::vector<MessageInfo> GetHistoryMsg(const std::string& chat_session_id, int64_t start_time,
        int64_t over_time, const std::string& token);
    
    std::vector<MessageInfo> GetRecentMsg(const std::string& chat_session_id, int64_t msg_count,
        int64_t cur_time, const std::string& token);
    
    std::vector<MessageInfo> MsgSearch(const std::string& chat_session_id, const std::string& search_key,
        const std::string& token);
private:
    MessageDao::Ptr dao_;
    
    SvcDiscovery::Ptr svc_discovery_;
    brpc::ChannelOptions channel_options_;
};
