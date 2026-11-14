#include "FriendService.h"
#include "user.pb.h"

FriendService::FriendService(FriendDao::Ptr dao, SvcDiscovery::Ptr service_discovery,
    const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
    int timeout_ms, int max_retry)
    : dao_(dao)
    , svc_discovery_(service_discovery)
    , etcd_server_ip_(etcd_server_ip)
    , protocol_(protocol)
    , connect_timeout_ms_(connect_timeout_ms)
    , timeout_ms_(timeout_ms)
    , max_retry_(max_retry)
{
    svc_discovery_->Watch(); // 拉取一次全部服务节点信息
    options_.max_retry = max_retry_;
    options_.connect_timeout_ms = connect_timeout_ms_;
    options_.timeout_ms = timeout_ms_;
    options_.protocol = protocol_;
}

std::vector<UserInfo> FriendService::GetMultiUserInfo_(std::vector<std::string>& users_id){
    std::string ip;
    auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
    if(channel->Init(etcd_server_ip_.c_str(), &options_)){
        LOG_ERROR("Init channel failed, ip: {}", etcd_server_ip_);
        return {};
    }

    ::brpc::Controller controller;
    ::linkchat::UserService_Stub stub(channel.get());
    ::linkchat::GetMultiUserInfoReq request;
    ::linkchat::GetMultiUserInfoRsp response;
}

std::vector<UserInfo> FriendService::GetUserInfo_(const std::string& user_id){

}