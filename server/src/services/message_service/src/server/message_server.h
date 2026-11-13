// 用来提供消息存储（消费 rabbitmq）、存储检索（brpc 服务器）
//
// 需要提供的数据服务：
// 1. 消息元数据存入数据库
// 2. 文件、图片存入oss，存入成功后，更新数据库
// 3. 文本信息直接存入 ES
// 4. 提供 ES、数据库 检索接口
//    GetHistoryMsg，获取指定时间段的消息
//    GetRecentMsg，获取最近 N 条消息
//    MsgSearch，检索文本消息
// !!!! 需构建项目，在 entity/sql 生成 .sql 建表文件，需要建库建表后运行


#pragma once
#include <brpc/server.h>
#include <butil/logging.h>

#include "message.pb.h"
#include "service/message_manager.h"
#include "google/protobuf/util/json_util.h"

using namespace linkchat;
class MessageServer: public MsgStorageService{
public:
    MessageServer(
        const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
                int timeout_ms, int max_retry,
                const char* user, const char* passwd, const char* db_name = "linkchat", const char* host = "mysql-service",
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