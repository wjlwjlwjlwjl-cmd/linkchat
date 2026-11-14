#include "user_controller.h"
#include "base.pb.h"
#include <brpc/closure_guard.h>

UserController::UserController(UserManager::Ptr user_manager){
    user_manager_ = user_manager;
}

void UserController::UserRegister(::google::protobuf::RpcController* controller,
                    const ::linkchat::UserRegisterReq* request,
                    ::linkchat::UserRegisterRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    CommonDTO ret = user_manager_->Register(request->nickname(), request->password(), request->email(), request->verify_code());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}

void UserController::NicknameLogin(::google::protobuf::RpcController* controller,
                    const ::linkchat::NicknameLoginReq* request,
                    ::linkchat::NicknameLoginRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    LoginDTO ret = user_manager_->NicknameLogin(request->nickname(), request->password());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
    response->set_token(ret.token);
}

void UserController::EmailLogin(::google::protobuf::RpcController* controller,
                    const ::linkchat::EmailLoginReq* request,
                    ::linkchat::EmailLoginRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    LoginDTO ret = user_manager_->EmailLogin(request->email(), request->verify_code());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
    response->set_token(ret.token);
}

void UserController::GetEmailVerifyCode(::google::protobuf::RpcController* controller,
                    const ::linkchat::EmailVerifyCodeReq* request,
                    ::linkchat::EmailVerifyCodeRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    CommonDTO ret = user_manager_->SendVerifyCode(request->email());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}

void UserController::GetUserInfo(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetUserInfoReq* request,
                    ::linkchat::GetUserInfoRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    std::string user_id;
    if(!JwtParse(request->token(), "user_id", user_id) || user_id.empty()){
        response->set_success(false);
        response->set_errmsg("token Invalid");
        return;
    }
    auto info = user_manager_->GetUserInfo(user_id); // 获取 user_id 的 
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

void UserController::GetMultiUserInfo(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetMultiUserInfoReq* request,
                    ::linkchat::GetMultiUserInfoRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    auto mems = request->users_id();
    std::vector<std::string> users_id(mems.begin(), mems.end());

    auto users_info = response->mutable_users_info();

    for(auto user_id: users_id){ // users_id 是所有要获取的
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

void UserController::SetUserAvatar(::google::protobuf::RpcController* controller,
                    const ::linkchat::SetUserAvatarReq* request,
                    ::linkchat::SetUserAvatarRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);

    // 设置谁的，从 token 中获取
    std::string token = request->token();
    std::string user_id;
    if(token.empty() || !JwtParse(token, "user_id", user_id)){
        response->set_success(false);
        response->set_errmsg("token Invalid");
        return;
    }
    CommonDTO ret = user_manager_->SetUserAvatar(user_id, request->avatar());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}

void UserController::SetUserNickname(::google::protobuf::RpcController* controller,
                    const ::linkchat::SetUserNicknameReq* request,
                    ::linkchat::SetUserNicknameRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);

    std::string token = request->token();
    std::string user_id;
    if(token.empty() || !JwtParse(token, "user_id", user_id)){
        response->set_success(false);
        response->set_errmsg("token Invalid");
        return;
    }
    CommonDTO ret = user_manager_->SetUserNickname(user_id, request->nickname());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}

void UserController::SetUserDescription(::google::protobuf::RpcController* controller,
                    const ::linkchat::SetUserDescriptionReq* request,
                    ::linkchat::SetUserDescriptionRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);

    std::string token = request->token();
    std::string user_id;
    if(token.empty() || !JwtParse(token, "user_id", user_id)){
        response->set_success(false);
        response->set_errmsg("token Invalid");
        return;
    }
    CommonDTO ret = user_manager_->SetUserDescription(user_id, request->description());
    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
}
