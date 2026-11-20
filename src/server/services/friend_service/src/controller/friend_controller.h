#pragma once
#include "service/friend_manager.h"
#include "friend.pb.h"

class FriendController: public ::linkchat::FriendService{
public:
    FriendController(::FriendManager::Ptr friend_service);

    virtual void GetFriendList(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetFriendListReq* request,
                        ::linkchat::GetFriendListRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void FriendRemove(::google::protobuf::RpcController* controller,
                        const ::linkchat::FriendRemoveReq* request,
                        ::linkchat::FriendRemoveRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void FriendAdd(::google::protobuf::RpcController* controller,
                        const ::linkchat::FriendAddReq* request,
                        ::linkchat::FriendAddRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void FriendAddProcess(::google::protobuf::RpcController* controller,
                        const ::linkchat::FriendAddProcessReq* request,
                        ::linkchat::FriendAddProcessRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void GetPendingFriendEventList(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetPendingFriendEventListReq* request,
                        ::linkchat::GetPendingFriendEventListRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void FriendSearch(::google::protobuf::RpcController* controller,
                        const ::linkchat::FriendSearchReq* request,
                        ::linkchat::FriendSearchRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void GetChatSessionList(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetChatSessionListReq* request,
                        ::linkchat::GetChatSessionListRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void ChatSessionCreate(::google::protobuf::RpcController* controller,
                        const ::linkchat::ChatSessionCreateReq* request,
                        ::linkchat::ChatSessionCreateRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void GetChatSessionMember(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetChatSessionMemberReq* request,
                        ::linkchat::GetChatSessionMemberRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void GetSingleChatSession(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetSingleChatSessionReq* request,
                        ::linkchat::GetSingleChatSessionRsp* response,
                        ::google::protobuf::Closure* done) override;
private:
    ::FriendManager::Ptr friend_service_;
};
