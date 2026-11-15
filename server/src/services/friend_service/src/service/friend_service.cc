#include "friend_service.h"
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

::linkchat::GetFriendListRsp FriendService::GetFriendList(const std::string& user_id){
    ::linkchat::GetFriendListRsp response;
    auto dto = dao_->GetFriendList(user_id);
    if(!dto.success){
        LOG_ERROR("get friend list failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("get friend list failed");
        return response;
    }

    auto friends_id = dto.friend_ids;
    auto friends_info = GetMultiUserInfo_(friends_id);
    std::vector<::linkchat::UserInfo> infos;
    if(!friends_info.has_value()){
        response.set_success(false);
        response.set_errmsg("get friend info failed");
        return response;
    }
    std::transform(friends_info.value().begin(), friends_info.value().end(),
        std::back_inserter(infos), [&](const auto& kv){
        return kv.second;
    });
    response.mutable_friend_list()->Add(infos.begin(), infos.end());

    response.set_success(true);
    return response;
}

::linkchat::FriendRemoveRsp FriendService::FriendRemove(const std::string& user_id, const std::string& peer_id){
    ::linkchat::FriendRemoveRsp response;
    auto dto = dao_->FriendRemove(user_id, peer_id);
    if(!dto.success){
        LOG_ERROR("remove friend failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("remove friend failed");
        return response;
    }
    response.set_success(dto.success);
    return response;
}

::linkchat::FriendAddRsp FriendService::FriendAdd(const std::string& user_id, const std::string& respondent_id){
    ::linkchat::FriendAddRsp response;
    auto dto = dao_->FriendAdd(user_id, respondent_id);
    if(!dto.success){
        LOG_ERROR("add friend failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("add friend failed");
        return response;
    }
    response.set_success(true);
    response.set_notify_event_id(dto.notify_event_id);
    return response;
}

::linkchat::FriendAddProcessRsp FriendService::FriendAddProcess(const std::string& notify_event_id, bool agree, 
    const std::string& apply_user_id, const std::string& user_id){
    ::linkchat::FriendAddProcessRsp response;

    // 这里还需要获取 apply_user_name 作为单聊会话名称
    auto user_info = GetUserInfo_(user_id);
    if(!user_info.has_value()){
        response.set_success(false);
        response.set_errmsg("user not found");
        return response;
    }
    std::string apply_user_name = user_info.value().nickname();

    auto dto = dao_->FriendAddProcess(notify_event_id, agree, apply_user_id, user_id, apply_user_name);
    if(!dto.success){
        LOG_ERROR("friend add process failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("friend add process failed");
        return response;
    }
    response.set_success(true);
    if(agree){
        response.set_new_session_id(dto.new_session_id);
    }
    return response;
}

/// 私有方法
std::optional<std::unordered_map<std::string, ::linkchat::UserInfo>> 
    FriendService::GetMultiUserInfo_(std::vector<std::string>& users_id){
    std::string ip;
    auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
    if(channel->Init(etcd_server_ip_.c_str(), &options_)){
        LOG_ERROR("Init channel failed, ip: {}", etcd_server_ip_);
        return std::nullopt;
    }

    std::optional<std::unordered_map<std::string, ::linkchat::UserInfo>> ret;

    ::brpc::Controller controller;
    ::linkchat::UserService_Stub stub(channel.get());
    ::linkchat::GetMultiUserInfoReq request;
    ::linkchat::GetMultiUserInfoRsp response;
    request.mutable_users_id()->Add(users_id.begin(), users_id.end());
    stub.GetMultiUserInfo(&controller, &request, &response, nullptr);
    if(controller.Failed()){
        LOG_ERROR("Failed to call user_service: ", controller.ErrorText());
        return std::nullopt;
    }
    if(!response.success()){
        LOG_ERROR("Call user_service fail: ", response.errmsg());
        return std::nullopt;
    }
    for(auto& e: response.users_info()){
        ret.value()[e.first] = e.second;
    }
    return ret;
}

std::optional<::linkchat::UserInfo> FriendService::GetUserInfo_(const std::string& user_id){
    std::string ip;
    auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
    if(channel->Init(etcd_server_ip_.c_str(), &options_)){
        LOG_ERROR("Init channel failed, ip: {}", etcd_server_ip_);
        return {};
    }

    ::brpc::Controller controller;
    ::linkchat::UserService_Stub stub(channel.get());
    ::linkchat::GetUserInfoReq request;
    ::linkchat::GetUserInfoRsp response;
    request.set_user_id(user_id);
    stub.GetUserInfo(&controller, &request, &response, nullptr);
    if(controller.Failed()){
        LOG_ERROR("Failed to call user_service: ", controller.ErrorText());
        return std::nullopt;
    }
    if(!response.success()){
        LOG_ERROR("Call user_service fail: ", response.errmsg());
        return std::nullopt;
    }
    return response.user_info();
}