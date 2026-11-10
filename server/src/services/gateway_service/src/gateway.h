// 现在用户信息，统一存放在 jwt token 中，jwt token 首先用于鉴权，后续用于传递处理用户信息

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

class Gateway{
public:
    //FileService
    #define GET_SINGLE_FILE "/get_single_file"

    #define GET_EMAIL_VERIFY_CODE "/get_email_verify_code"

    Gateway(std::shared_ptr<::brpc::ChannelOptions> options, SvcDiscovery::Ptr svc_discovery, 
        redis_factory::RedisConnectionInfo& redis_connection_info){
        options_ = options;
        svc_discovery_ = svc_discovery;
        redis_template_ = redis_factory::RedisFactory::Create(redis_connection_info);

        // 拉取服务信息到本地
        svc_discovery_->Watch();

        http_server_.Get(GET_SINGLE_FILE, 
            std::bind(&Gateway::GetSingleFile, this, std::placeholders::_1, std::placeholders::_2));

        http_server_.Get(GET_EMAIL_VERIFY_CODE, 
            std::bind(&Gateway::GetEmailVerifyCode, this, std::placeholders::_1, std::placeholders::_2));
    }

    void run(const char* listen_ip, int port){
        LOG_INFO("Gateway is listening {} on {}", listen_ip, port);
        http_server_.listen(listen_ip, port);
    }
private:
    httplib::Server http_server_;
    std::shared_ptr<sw::redis::Redis> redis_template_;

    // 服务器通信相关
    std::shared_ptr<::brpc::ChannelOptions> options_;
    SvcDiscovery::Ptr svc_discovery_;

    bool verifyToken(const std::string& token){
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

    void GetSingleFile(const httplib::Request& req, httplib::Response& res){
        if(!verifyToken(req.get_header_value("Authorization"))){
            res.status = 401;
            return;
        }
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
        request.set_file_id("file_id");
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
};