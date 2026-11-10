#include "user_server.h"
#include "base.pb.h"
#include <brpc/closure_guard.h>

UserServer::UserServer(UserManager::Ptr user_manager){
    user_manager_ = user_manager;
}

void UserServer::UserRegister(::google::protobuf::RpcController* controller,
                    const ::linkchat::UserRegisterReq* request,
                    ::linkchat::UserRegisterRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    CommonDTO ret = user_manager_->Register(request->nickname(), request->password(), request->email(), request->verify_code());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}

void UserServer::NicknameLogin(::google::protobuf::RpcController* controller,
                    const ::linkchat::NicknameLoginReq* request,
                    ::linkchat::NicknameLoginRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    LoginDTO ret = user_manager_->NicknameLogin(request->nickname(), request->password());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
    response->set_jwt_token(ret.token);
}

void UserServer::EmailLogin(::google::protobuf::RpcController* controller,
                    const ::linkchat::EmailLoginReq* request,
                    ::linkchat::EmailLoginRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    LoginDTO ret = user_manager_->EmailLogin(request->email(), request->verify_code());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
    response->set_jwt_token(ret.token);
}

void UserServer::GetEmailVerifyCode(::google::protobuf::RpcController* controller,
                    const ::linkchat::EmailVerifyCodeReq* request,
                    ::linkchat::EmailVerifyCodeRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    CommonDTO ret = user_manager_->SendVerifyCode(request->email());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}

void UserServer::GetUserInfo(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetUserInfoReq* request,
                    ::linkchat::GetUserInfoRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto info = user_manager_->GetUserInfo(request->user_id());
    response->set_success(true);
    response->set_errmsg("");

    ::linkchat::UserInfo user_info;
    user_info.set_user_id(info.user_id);
    user_info.set_nickname(info.nickname);
    user_info.set_email(info.email);
    user_info.set_avatar(info.avatar_id);
    user_info.set_description(info.description);
    response->mutable_user_info()->CopyFrom(user_info);
}

void UserServer::GetMultiUserInfo(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetMultiUserInfoReq* request,
                    ::linkchat::GetMultiUserInfoRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto mems = request->users_id();
    std::vector<std::string> users_id(mems.begin(), mems.end());

    auto users_info = response->mutable_users_info();

    for(auto user_id: users_id){
        auto info = user_manager_->GetUserInfo(user_id);
        ::linkchat::UserInfo user_info;
        user_info.set_user_id(info.user_id);
        user_info.set_nickname(info.nickname);
        user_info.set_email(info.email);
        user_info.set_avatar(info.avatar_id);
        user_info.set_description(info.description);

        users_info->insert({user_id, user_info});
    }
    response->set_success(true);
    response->set_errmsg("");
}

void UserServer::SetUserAvatar(::google::protobuf::RpcController* controller,
                    const ::linkchat::SetUserAvatarReq* request,
                    ::linkchat::SetUserAvatarRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    CommonDTO ret = user_manager_->SetUserAvatar(request->user_id(), request->avatar());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}

void UserServer::SetUserNickname(::google::protobuf::RpcController* controller,
                    const ::linkchat::SetUserNicknameReq* request,
                    ::linkchat::SetUserNicknameRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    CommonDTO ret = user_manager_->SetUserNickname(request->user_id(), request->nickname());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}

void UserServer::SetUserDescription(::google::protobuf::RpcController* controller,
                    const ::linkchat::SetUserDescriptionReq* request,
                    ::linkchat::SetUserDescriptionRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    CommonDTO ret = user_manager_->SetUserDescription(request->user_id(), request->description());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}
