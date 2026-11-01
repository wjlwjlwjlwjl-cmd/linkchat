#include "message_server.h"
#include "base.pb.h"

MessageServer::MessageServer(
    const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
            int timeout_ms, int max_retry,
            const char* user, const char* passwd, const char* db_name, const char* host,
            unsigned int port, int connection_pool_size,
            const char* elastic_url,
            const char* index_name, const char* doc_type, const char* id
){
    manager_ = std::make_shared<MessageManager>(etcd_server_ip, protocol, connect_timeout_ms,
        timeout_ms, max_retry, user, passwd, db_name, host, port, connection_pool_size, elastic_url,
        index_name, doc_type);
}

void MessageServer::GetHistoryMsg(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetHistoryMsgReq* request,
                    ::linkchat::GetHistoryMsgRsp* response,
                    ::google::protobuf::Closure* done){
                        
}

void MessageServer::GetRecentMsg(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetRecentMsgReq* request,
                    ::linkchat::GetRecentMsgRsp* response,
                    ::google::protobuf::Closure* done){
                        
}

void MessageServer::MsgSearch(::google::protobuf::RpcController* controller,
                    const ::linkchat::MsgSearchReq* request,
                    ::linkchat::MsgSearchRsp* response,
                    ::google::protobuf::Closure* done){
                        
}

bool MessageServer::OnMessage(const char* message, int size){
    std::string raw = std::string(message, size);
    ::linkchat::MessageInfo message_info;
    auto status =::google::protobuf::util::JsonStringToMessage(raw, &message_info);
    if(!status.ok()){
        LOG_ERROR("Failed to deserialize message");
        return false;
    }
    manager_->PersistMessage(message_info);
    return true;
}