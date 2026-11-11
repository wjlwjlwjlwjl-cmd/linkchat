// 现在用户信息，统一存放在 jwt token 中，jwt token 首先用于鉴权，后续用于传递处理用户信息
// Get 方法参数，直接获取即可
// Post 方法参数，如果没有文件操作，body 为 application/json
// 如果有文件操作，body 为 multipart/form-data

#include <brpc/controller.h>
#include <httplib.h>
#include <string>
#include <brpc/channel.h>
#include <functional>
#include <google/protobuf/util/json_util.h>

#include "user.pb.h"
#include "file.pb.h"
#include "message.pb.h"
#include "logging/logger.h"
#include "etcd/discover/service_discovery.h"
#include "service_constants.h"
#include "jwt_util.h"
#include "redis/redis_factory.h"
#include "redis_constants.h"
#include "json/json_codec.h"

class Gateway{
public:
    //FileService
    #define GET_SINGLE_FILE "/file/get_single"

    // UserService
    #define GET_EMAIL_VERIFY_CODE "/login/mail_verify_code"
    #define REGISTER "/register"
    #define NICKNAME_LOGIN "/login/nickname"
    #define EMAIL_LOGIN "/login/email"
    #define GET_USER_INFO "/user/info"
    #define GET_MULTI_USER_INFO "/user/multi_info"
    #define SET_USER_AVATAR "/user/avatar/set"
    #define SET_USER_DESCRIPTION "/user/description/set"
    #define SET_USER_NICKNAME "/user/nickname/set"

    Gateway(std::shared_ptr<::brpc::ChannelOptions> options, SvcDiscovery::Ptr svc_discovery, 
        redis_factory::RedisConnectionInfo& redis_connection_info){
        options_ = options;
        svc_discovery_ = svc_discovery;
        redis_template_ = redis_factory::RedisFactory::Create(redis_connection_info);

        // 设置网关统一身份验证
        http_server_.set_pre_routing_handler([this](const ::httplib::Request& req, ::httplib::Response& res){
            // 处理跨域时浏览器的预检请求
            if (req.method == "OPTIONS") {
                res.set_header("Access-Control-Allow-Origin", "*"); 
                res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
                res.status = 204;
                return ::httplib::Server::HandlerResponse::Handled; // 直接结束
            }

            // 白名单直接放行
            if(white_lists.find(req.path) != white_lists.end()){
                return ::httplib::Server::HandlerResponse::Unhandled;
            }

            // 验证登录态
            std::string token = req.get_header_value("Authorization");
            if(token.empty()){
                res.status = 401;
                return ::httplib::Server::HandlerResponse::Handled;
            }
            if(!VerifyToken(token)){
                res.status = 401;
                return ::httplib::Server::HandlerResponse::Handled;
            }
            return ::httplib::Server::HandlerResponse::Unhandled;
        });

        // 拉取服务信息到本地
        svc_discovery_->Watch();

        ///////////////////////////////
        // FileService
        ///////////////////////////////
        http_server_.Get(GET_SINGLE_FILE, 
            std::bind(&Gateway::GetSingleFile, this, std::placeholders::_1, std::placeholders::_2));

        ///////////////////////////////
        // UserService
        ///////////////////////////////
        http_server_.Get(GET_EMAIL_VERIFY_CODE, 
            std::bind(&Gateway::GetEmailVerifyCode, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Post(REGISTER, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                Register(req, res);
            });
        http_server_.Post(NICKNAME_LOGIN, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                NicknameLogin(req, res);
            });
        http_server_.Post(EMAIL_LOGIN, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                EmailLogin(req, res);
            });
        http_server_.Get(GET_USER_INFO, 
            std::bind(&Gateway::GetUserInfo, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Get(GET_MULTI_USER_INFO, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                GetMultiUserInfo(req, res);
            });
        http_server_.Post(SET_USER_AVATAR, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                SetUserAvatar(req, res);
            });
        http_server_.Post(SET_USER_DESCRIPTION, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                SetUserDescription(req, res);
            });
        http_server_.Post(SET_USER_NICKNAME, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                SetUserNickname(req, res);
            });
    }

    void run(const char* listen_ip, int port){
        LOG_INFO("Gateway is listening {} on {}", listen_ip, port);
        http_server_.listen(listen_ip, port);
    }
private:
    // 白名单接口不校验token
    const std::set<std::string> white_lists{
        GET_EMAIL_VERIFY_CODE,
        REGISTER,
        NICKNAME_LOGIN,
        EMAIL_LOGIN,
    };

    httplib::Server http_server_;
    std::shared_ptr<sw::redis::Redis> redis_template_;

    // 服务器通信相关
    std::shared_ptr<::brpc::ChannelOptions> options_;
    SvcDiscovery::Ptr svc_discovery_;

    bool VerifyToken(const std::string& token){
        // 这里检查 jwt token 能否成功解析，能则认为jwt合法，如果不合法，直接 401 Unauthorized
        if(token.empty()) return false;
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            return false;
        }
        if(user_id == "") return false;

        // jwt 合法，检查是否过期，存在则“续租”
        std::string token_key = redis_constants::token_key + user_id;
        if(!redis_template_->exists(token_key)){
            return false;
        }
        redis_template_->expire(token_key, redis_constants::token_expire);
        return true;
    }

    ///////////////////////////////
    // FileService
    ///////////////////////////////
    void GetSingleFile(const httplib::Request& req, httplib::Response& res){
        std::string file_id = req.get_param_value("file_id");
        if(file_id.empty()){
            res.status = 400;
            return;
        }

        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::file_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::file_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::file_service, ip);
            res.status = 502;
            return;
        }
        ::brpc::Controller cntl;
        ::linkchat::FileService_Stub stub(channel.get());

        ::linkchat::GetSingleFileReq request;
        ::linkchat::GetSingleFileRsp response;
        request.set_file_id(file_id);
        stub.GetSingleFile(&cntl, &request, &response, nullptr);

        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get single file response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    ///////////////////////////////
    // UserService
    ///////////////////////////////
    void Register(const ::httplib::Request& req, ::httplib::Response& res){
        // 与 brpc 服务器建立连接
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }
        ::linkchat::UserService_Stub stub(channel.get());

        ::linkchat::UserRegisterReq request;
        ::linkchat::UserRegisterRsp response;
        std::string body = req.body;
        auto status = ::google::protobuf::util::JsonStringToMessage(body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }

        ::brpc::Controller cntl;
        stub.UserRegister(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize register response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    void GetEmailVerifyCode(const httplib::Request& req, httplib::Response& res){
        // 与 brpc 服务器建立连接
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }
        ::linkchat::UserService_Stub stub(channel.get());

        std::string email = req.get_param_value("email"); // 获取路径参数

        // 调用用户服务获取用户信息
        ::brpc::Controller cntl;
        ::linkchat::EmailVerifyCodeReq request;
        request.set_email(email);
        ::linkchat::EmailVerifyCodeRsp response;
        stub.GetEmailVerifyCode(&cntl, &request, &response, nullptr);

        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize email verify code response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    void NicknameLogin(const ::httplib::Request& req, ::httplib::Response& res){
        // 与 brpc 服务器建立连接
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }
        ::linkchat::UserService_Stub stub(channel.get());
        
        ::linkchat::NicknameLoginReq request;
        ::linkchat::NicknameLoginRsp response;
        std::string body = req.body;
        auto status = ::google::protobuf::util::JsonStringToMessage(body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }

        ::brpc::Controller cntl;
        stub.NicknameLogin(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize nickname login response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    void EmailLogin(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }
        ::linkchat::UserService_Stub stub(channel.get());
        
        ::linkchat::EmailLoginReq request;
        ::linkchat::EmailLoginRsp response;
        std::string body = req.body;
        auto status = ::google::protobuf::util::JsonStringToMessage(body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }

        ::brpc::Controller cntl;
        stub.EmailLogin(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize email login response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    void GetUserInfo(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::GetUserInfoReq request;
        ::linkchat::GetUserInfoRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        request.set_token(token);
        ::brpc::Controller cntl;
        stub.GetUserInfo(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get info response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    void GetMultiUserInfo(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::GetMultiUserInfoReq request;
        ::linkchat::GetMultiUserInfoRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            std::cout << "token empty" << std::endl;
            res.status = 401;
            return;
        }
        auto ids = req.params.equal_range("users_id");
        for(auto it = ids.first; it != ids.second; ++it){
            request.add_users_id(it->second);
        }
        if(request.users_id_size() == 0){
            res.status = 400;
            return;
        }
        request.set_token(token);
        ::brpc::Controller cntl;
        stub.GetMultiUserInfo(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    void SetUserAvatar(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::SetUserAvatarReq request;
        ::linkchat::SetUserAvatarRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        request.set_token(token);
        if(!req.has_file("avatar")){
            res.status = 400;
            return;
        }
        const auto& avatar_content = req.get_file_value("avatar");
        std::string avatar = avatar_content.content;
        request.set_avatar(avatar);
        ::brpc::Controller cntl;
        stub.SetUserAvatar(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    void SetUserDescription(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::SetUserDescriptionReq request;
        ::linkchat::SetUserDescriptionRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        request.set_token(token);
        std::string description = req.get_param_value("description");
        if(description.empty()){
            res.status = 400;
            return;
        }
        request.set_description(description);
        ::brpc::Controller cntl;
        stub.SetUserDescription(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    void SetUserNickname(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::SetUserNicknameReq request;
        ::linkchat::SetUserNicknameRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        request.set_token(token);
        std::string nickname = req.get_param_value("nickname");
        if(nickname.empty()){
            res.status = 400;
            return;
        }
        request.set_nickname(nickname);
        ::brpc::Controller cntl;
        stub.SetUserNickname(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }
};