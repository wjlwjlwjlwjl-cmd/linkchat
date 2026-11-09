#include "user_manager.h"
#include "logging/logger.h"
#include "random_id.h"

UserManager::UserManager(UserDao::Ptr dao, redis_factory::RedisConnectionInfo redis_info, 
    const std::string& host, const std::string& user, const std::string& pass,
    const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
    int timeout_ms, int max_retry
){
    dao_ = dao;

    redis_factory::RedisFactory redis_factory;
    redis_template_ = redis_factory.Create(redis_info);

    smtp_host_ = host;
    smtp_user_ = user;
    smtp_pass_ = pass;

    etcd_server_ip_ = etcd_server_ip;
    protocol_ = protocol;
    connect_timeout_ms_ = connect_timeout_ms;
    timeout_ms_ = timeout_ms;
    max_retry_ = max_retry;
    channel_options_.connect_timeout_ms = connect_timeout_ms;
    channel_options_.max_retry = max_retry;
    channel_options_.timeout_ms = timeout_ms;
    channel_options_.protocol = protocol;
    SvcDiscovery::ModCallback cb_online = std::function<void(const std::string&, const std::string&)>();
    SvcDiscovery::ModCallback cb_offline = std::function<void(const std::string&, const std::string&)>();
    svc_discovery_ = std::make_shared<SvcDiscovery>(etcd_server_ip, std::move(cb_online), std::move(cb_offline));
    svc_discovery_->Watch(); // 到 etcd 全量获取一次服务-节点信息
}

CommonDTO UserManager::Register(const std::string &nickname, const std::string &password, 
    const std::string &email, const std::string &verify_code) {
    //检查验证码是否有效
    auto verify_code_key = redis_constants::email_verify_code_key + email;
    auto stored_code = redis_template_->get(verify_code_key);
    if(!stored_code.has_value() || stored_code.value() != verify_code){
        return {false, "Verify code is incorrect"};
    }
    std::string ret = dao_->Register(nickname, password, email);
    return {ret.empty(), ret};
}

LoginDTO UserManager::NicknameLogin(const std::string &nickname, const std::string &password){
    std::string jwt_token;

    auto user = dao_->GetUserByNickname(nickname);
    std::string user_id = user.user_id;
    if(user_id.empty()){
        return {false, "User not found", ""};
    }

    jwt_token = JwtCreate("user_id", user_id);

    return LoginDTO {true, "", jwt_token};
}

LoginDTO UserManager::EmailLogin(const std::string &email, const std::string &verify_code){
    auto verify_code_key = redis_constants::email_verify_code_key + email;
    auto stored_code = redis_template_->get(verify_code_key);
    if(!stored_code.has_value() || stored_code.value() != verify_code){
        return {false, "Verify code is incorrect", ""};
    }

    std::string jwt_token;
    auto user = dao_->GetUserByEmail(email);
    std::string user_id = user.user_id;
    if(user_id.empty()){
        return {false, "User not found", ""};
    }

    jwt_token = JwtCreate("user_id", user_id);
    return {true, "", jwt_token};
}

CommonDTO UserManager::SendVerifyCode(const std::string &email){
    std::string verify_code = random_util::GetVerifyCode();

    // 发送邮件验证码
    mail::MailMessage mail = {
        .from = smtp_user_,
        .to = email,
        .subject = "LinkChat Verify Code",
        .body = "Your verify code is: " + verify_code
    };
    if(!mail::SendMail(smtp_host_, smtp_user_, smtp_pass_, mail)){
        LOG_WARN("Failed to send email");
        return {false, "Failed to send verify code email"};
    }

    auto verify_code_key = redis_constants::email_verify_code_key + email;
    if(!redis_template_->set(verify_code_key, verify_code, redis_constants::email_verify_code_expire)){
        LOG_WARN("Failed to set verify code in redis");
        return {false, "Please try again later"};
    }

    return {true, ""};
}

UserDTO UserManager::GetUserInfo(const std::string &user_id){
    return dao_->GetUserById(user_id);
}

CommonDTO UserManager::SetUserAvatar(const std::string& user_id, const std::string& content){
    // 这里调用文件存储子服务，上传头像后，在数据库保存 file_id，获取头像时，获取到的是 file_id，后续自己到文件存储自服务获取预签名即可
    std::string ip;
    ChannelPtr channel = svc_discovery_->GetSvcChannel(service_name::file_service, ip);

    if(!channel->Init(ip.c_str(), &channel_options_)){
        LOG_ERROR("Failed to init channel");
        return {false, "Failed to init channel"};
    }
    ::linkchat::FileService_Stub stub(channel.get());

    ::linkchat::PutSingleFileReq req;

    std::string file_name = service_prefix::user_avatar_prefix + user_id;
    std::string file_content = "";
    req.mutable_file_data()->set_file_name(file_name);
    req.mutable_file_data()->set_file_content(file_content);

    std::unique_ptr<brpc::Controller> controller(new ::brpc::Controller());
    std::unique_ptr<::linkchat::PutSingleFileRsp> response(new ::linkchat::PutSingleFileRsp());
    stub.PutSingleFile(controller.get(), &req, response.get(), nullptr);

    if(controller->Failed()){
        LOG_ERROR("Failed to put file");
        return {false, "Failed to put file"};
    }
    std::string file_id = response->file_info().file_id();
    dao_->SetUserAvatar(user_id, file_id);
    return {true, ""};
}

CommonDTO UserManager::SetUserDescription(const std::string& user_id, const std::string& description){
    auto ret = dao_->SetUserDescription(user_id, description);
    return {ret.empty(), ret};
}

CommonDTO UserManager::SetUserNickname(const std::string& user_id, const std::string& nickname){
    auto ret = dao_->SetUserNickname(user_id, nickname);
    return {ret.empty(), ret};
}