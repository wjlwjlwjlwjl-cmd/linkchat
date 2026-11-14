#pragma once
#include "service/user_manager.h"
#include "user.pb.h"
#include "domain/login_dto.h"
#include "domain/user_dto.h"

class UserController: public ::linkchat::UserService{
public:
    UserController(UserManager::Ptr user_manager);

    virtual void UserRegister(::google::protobuf::RpcController* controller,
                        const ::linkchat::UserRegisterReq* request,
                        ::linkchat::UserRegisterRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void NicknameLogin(::google::protobuf::RpcController* controller,
                        const ::linkchat::NicknameLoginReq* request,
                        ::linkchat::NicknameLoginRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void EmailLogin(::google::protobuf::RpcController* controller,
                        const ::linkchat::EmailLoginReq* request,
                        ::linkchat::EmailLoginRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void GetEmailVerifyCode(::google::protobuf::RpcController* controller,
                        const ::linkchat::EmailVerifyCodeReq* request,
                        ::linkchat::EmailVerifyCodeRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void GetUserInfo(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetUserInfoReq* request,
                        ::linkchat::GetUserInfoRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void GetMultiUserInfo(::google::protobuf::RpcController* controller,
                        const ::linkchat::GetMultiUserInfoReq* request,
                        ::linkchat::GetMultiUserInfoRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void SetUserAvatar(::google::protobuf::RpcController* controller,
                        const ::linkchat::SetUserAvatarReq* request,
                        ::linkchat::SetUserAvatarRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void SetUserNickname(::google::protobuf::RpcController* controller,
                        const ::linkchat::SetUserNicknameReq* request,
                        ::linkchat::SetUserNicknameRsp* response,
                        ::google::protobuf::Closure* done) override;
    virtual void SetUserDescription(::google::protobuf::RpcController* controller,
                        const ::linkchat::SetUserDescriptionReq* request,
                        ::linkchat::SetUserDescriptionRsp* response,
                        ::google::protobuf::Closure* done) override;
private:
    UserManager::Ptr user_manager_;
};