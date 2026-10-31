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

using namespace linkchat;
class MessageServer: public MsgStorageService{
public:
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
private:
};