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

class Gateway{
public:
    #define GET_EMAIL_VERIFY_CODE "/get_email_verify_code"

    Gateway(std::shared_ptr<::brpc::ChannelOptions> options, SvcDiscovery::Ptr svc_discovery){
        options_ = options;
        svc_discovery_ = svc_discovery;

        // 拉取服务信息到本地
        svc_discovery_->Watch();

        http_server_.Get(GET_EMAIL_VERIFY_CODE, 
            std::bind(&Gateway::GetEmailVerifyCode, this, std::placeholders::_1, std::placeholders::_2));
    }

    void run(const char* listen_ip, int port){
        LOG_INFO("Gateway is listening {} on {}", listen_ip, port);
        http_server_.listen(listen_ip, port);
    }
private:
    httplib::Server http_server_;

    // 服务器通信相关
    std::shared_ptr<::brpc::ChannelOptions> options_;
    SvcDiscovery::Ptr svc_discovery_;

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