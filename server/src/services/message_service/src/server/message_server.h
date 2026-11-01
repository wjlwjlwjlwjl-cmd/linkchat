// 消息存储子服务
// 文件、语音、图片转交文件存储子服务
// 文本消息存储 ES（检索）
// 所有消息简息存到数据库
// 数据来源：订阅 rabbitmq 队列
//
// 暴露的 rpc 接口，主要是消息的搜索获取
// GetHistoryMsg，获取指定时间段的消息
// GetRecentMsg，获取最近 N 条消息
// MsgSearch，检索文本消息


#pragma once
#include <brpc/server.h>
#include <butil/logging.h>

#include "message.pb.h"
#include "service/message_manager.h"
#include "json/json_codec.h"
#include "google/protobuf/util/json_util.h"

using namespace linkchat;
class MessageServer: public MsgStorageService{
public:
    MessageServer(
        const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
                int timeout_ms, int max_retry,
                const char* user, const char* passwd, const char* db_name = "db_msg", const char* host = "mysql-service",
                unsigned int port = 3306, int connection_pool_size = 5,
                const char* elastic_url = "http://elastic:123456@elasticsearch-service:9200/",
                const char* index_name = "message", const char* doc_type = "_doc", const char* id = ""
    );
    
    void GetHistoryMsg(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetHistoryMsgReq* request,
                        ::linkchat::GetHistoryMsgRsp* response,
                        ::google::protobuf::Closure* done);
    
    void GetRecentMsg(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetRecentMsgReq* request,
                        ::linkchat::GetRecentMsgRsp* response,
                        ::google::protobuf::Closure* done);
    
    void MsgSearch(::google::protobuf::RpcController* controller,
                        const ::linkchat::MsgSearchReq* request,
                        ::linkchat::MsgSearchRsp* response,
                        ::google::protobuf::Closure* done);
    
    //监听rabbitmq回调函数，拿到消息后进行持久化
    bool OnMessage(const char* message, int size);
private:
    MessageManager::Ptr manager_;
};