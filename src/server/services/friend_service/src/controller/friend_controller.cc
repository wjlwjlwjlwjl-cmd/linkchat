#include "friend_controller.h"
#include "base.pb.h"
#include <brpc/closure_guard.h>

FriendController::FriendController(::FriendManager::Ptr friend_service){
    friend_service_ = friend_service;
}

void FriendController::GetFriendList(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetFriendListReq* request,
                    ::linkchat::GetFriendListRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto ret = friend_service_->GetFriendList(request->user_id());
    response->CopyFrom(ret);
}

void FriendController::FriendRemove(::google::protobuf::RpcController* controller,
                    const ::linkchat::FriendRemoveReq* request,
                    ::linkchat::FriendRemoveRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto ret = friend_service_->FriendRemove(request->user_id(), request->peer_id());
    response->CopyFrom(ret);
}

void FriendController::FriendAdd(::google::protobuf::RpcController* controller,
                    const ::linkchat::FriendAddReq* request,
                    ::linkchat::FriendAddRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto ret = friend_service_->FriendAdd(request->user_id(), request->respondent_id());
    response->CopyFrom(ret);
}

void FriendController::FriendAddProcess(::google::protobuf::RpcController* controller,
                    const ::linkchat::FriendAddProcessReq* request,
                    ::linkchat::FriendAddProcessRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto ret = friend_service_->FriendAddProcess(
        request->notify_event_id(), request->agree(),
        request->apply_user_id(), request->user_id());
    response->CopyFrom(ret);
}

void FriendController::GetPendingFriendEventList(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetPendingFriendEventListReq* request,
                    ::linkchat::GetPendingFriendEventListRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto ret = friend_service_->GetPendingFriendEventList(request->user_id());
    response->CopyFrom(ret);
}

void FriendController::FriendSearch(::google::protobuf::RpcController* controller,
                    const ::linkchat::FriendSearchReq* request,
                    ::linkchat::FriendSearchRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto ret = friend_service_->FriendSearch(request->search_key(), request->count());
    response->CopyFrom(ret);
}

void FriendController::GetChatSessionList(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetChatSessionListReq* request,
                    ::linkchat::GetChatSessionListRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto ret = friend_service_->GetChatSessionList(request->user_id());
    response->CopyFrom(ret);
}

void FriendController::ChatSessionCreate(::google::protobuf::RpcController* controller,
                    const ::linkchat::ChatSessionCreateReq* request,
                    ::linkchat::ChatSessionCreateRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);

    auto mems = request->member_id_list();
    std::vector<std::string> member_id_list(mems.begin(), mems.end());

    auto ret = friend_service_->ChatSessionCreate(
        request->user_id(), request->chat_session_name(), member_id_list);
    response->CopyFrom(ret);
}

void FriendController::GetChatSessionMember(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetChatSessionMemberReq* request,
                    ::linkchat::GetChatSessionMemberRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto ret = friend_service_->GetChatSessionMember(request->user_id(), request->chat_session_id());
    response->CopyFrom(ret);
}
