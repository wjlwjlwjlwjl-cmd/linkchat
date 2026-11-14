#pragma once
#include "dao/user_dao.h"

#include "domain/login_dto.h"
#include "redis/redis_factory.h"
#include "redis_constants.h"
#include "domain/user_dto.h"
#include "domain/common_dto.h"
#include "botan/argon2id.h"
#include "jwt_util.h"
#include "random_id.h"
#include "mail/mail_sender.h"
#include "base.pb.h"
#include "file.pb.h"
#include "etcd/discover/service_discovery.h"
#include "service_constants.h"
#include "elastic/elastic_client.h"

class UserManager{
public:
    using Ptr = std::shared_ptr<UserManager>;

    UserManager(UserDao::Ptr dao, redis_factory::RedisConnectionInfo redis_info, 
        const std::string& smtp_host, const std::string& smtp_user, const std::string& smtp_pass,
        const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
        int timeout_ms, int max_retry,
        const std::string& elastic_url, const std::string& index_name, const std::string& doc_type
    );

    /// @brief 用户注册
    /// @param nickname 昵称
    /// @param password 密码
    /// @param email 邮箱
    /// @param verify_code 验证码
    /// @return 通用DTO，success、errmsg
    CommonDTO Register(const std::string &nickname, const std::string &password, 
        const std::string &email, const std::string &verify_code);

    /// @brief 用户昵称登录
    /// @param nickname 昵称
    /// @param password 密码
    /// @return 登录DTO，success、errmsg、token
    LoginDTO NicknameLogin(const std::string &nickname, const std::string &password);

    /// @brief 用户邮箱登录
    /// @param email 邮箱
    /// @param verify_code 验证码
    /// @return 登录DTO，success、errmsg、token
    LoginDTO EmailLogin(const std::string &email, const std::string &verify_code);

    /// @brief 发送验证码
    /// @param email 邮箱
    /// @return 验证码DTO，success、errmsg
    CommonDTO SendVerifyCode(const std::string &email);

    /// @brief 获取用户信息
    /// @param user_id 用户ID
    /// @return 用户信息DTO，success、errmsg、user_info
    UserDTO GetUserInfo(const std::string &user_id);

    /// @brief 设置用户头像
    /// @param user_id 用户ID
    /// @param content 头像文件二进制内容
    /// @return 通用DTO，success、errmsg
    CommonDTO SetUserAvatar(const std::string& user_id, const std::string& content);

    /// @brief 设置用户描述
    /// @param user_id 用户ID
    /// @param description 描述
    /// @return 通用DTO，success、errmsg
    CommonDTO SetUserDescription(const std::string& user_id, const std::string& description);

    /// @brief 设置用户昵称
    /// @param user_id 用户ID
    /// @param nickname 昵称
    /// @return 通用DTO，success、errmsg
    CommonDTO SetUserNickname(const std::string& user_id, const std::string& nickname);
private:
    std::string smtp_host_;
    std::string smtp_user_;
    std::string smtp_pass_;

    std::string etcd_server_ip_;
    std::string protocol_;
    int connect_timeout_ms_;
    int timeout_ms_;
    int max_retry_;
    brpc::ChannelOptions channel_options_;
    SvcDiscovery::Ptr svc_discovery_;

    UserDao::Ptr dao_;
    std::shared_ptr<sw::redis::Redis> redis_template_;
    ElasticClient::Ptr es_;
};