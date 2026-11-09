#include "etcd/discover/service_discovery.h"
#include "service_constants.h"
#include "logging/logger.h"
#include "user.pb.h"

void RegisterTest(::brpc::Controller& controller, ::linkchat::UserService& stub){
    ::linkchat::UserRegisterReq req;
    req.set_request_id("123456");
    req.set_nickname("test_user");
    req.set_password("123456");
    req.set_email("test@example.com");
    req.set_verify_code("123456");

    ::linkchat::UserRegisterRsp rsp;

    stub.UserRegister(&controller, &req, &rsp, nullptr);
    std::cout << rsp.success() << std::endl;
    std::cout << rsp.errmsg() << std::endl;
}

int main(){
    InitLogger();
    SvcDiscovery::Ptr svc_discovery_;
    brpc::ChannelOptions channel_options_;

    channel_options_.connect_timeout_ms = 10000;
    channel_options_.max_retry = 3;
    channel_options_.timeout_ms = 100000;
    channel_options_.protocol = "baidu_std";
    SvcDiscovery::ModCallback cb_online = std::function<void(const std::string&, const std::string&)>();
    SvcDiscovery::ModCallback cb_offline = std::function<void(const std::string&, const std::string&)>();
    svc_discovery_ = std::make_shared<SvcDiscovery>("http://etcd-service:2379", std::move(cb_online), std::move(cb_offline));
    svc_discovery_->Watch(); // 到 etcd 全量获取一次服务-节点信息

    std::string ip;
    ChannelPtr channel = svc_discovery_->GetSvcChannel(service_name::file_service, ip);
    if (!channel) {
        LOG_ERROR("Failed to get channel to [{}]", service_name::file_service);
        return 1;
    }

    // brpc::Channel::Init 成功返回 0，失败返回 -1
    if(channel->Init(ip.c_str(), &channel_options_) != 0){
        LOG_ERROR("Failed to init channel to [{}]", ip.c_str());
        return 1;
    }
    LOG_INFO("channel init success");

    brpc::Controller controller;
    ::linkchat::UserService_Stub stub(channel.get());

    return 0;
}