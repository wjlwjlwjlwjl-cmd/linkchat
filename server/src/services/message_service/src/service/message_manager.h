#include "dao/message_dao.h"
#include "message.pb.h"
#include "file.pb.h"
#include "etcd/discover/service_discovery.h"
#include "service_constants.h"

class MessageManager{
public:
    using Ptr = std::shared_ptr<MessageManager>;
    MessageManager(
        const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
        int timeout_ms, int max_retry,
        const char* user, const char* passwd, const char* db_name = "msg_db", const char* host = "mysql-service",
        unsigned int port = 3306, int connection_pool_size = 5,
        const char* elastic_url = "http://elastic:123456@elasticsearch-service:9200/",
        const char* index_name = "message", const char* doc_type = "_doc", const char* id = "");

    //消息持久化
    bool PersistMessage(::linkchat::MessageInfo& info);

    // 三个 brpc 接口
    void GetHistoryMsg(const ::linkchat::GetHistoryMsgReq& request, ::linkchat::GetHistoryMsgRsp&);
    
    void GetRecentMsg(const ::linkchat::GetRecentMsgReq& request, ::linkchat::GetRecentMsgRsp&);
    
    void MsgSearch(const ::linkchat::MsgSearchReq& request, ::linkchat::MsgSearchRsp&);
private:
    MessageDao::Ptr dao_;
    
    SvcDiscovery::Ptr svc_discovery_;
    brpc::ChannelOptions channel_options_;
};
