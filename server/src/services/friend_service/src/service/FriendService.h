#pragma once

#include <brpc/server.h>

#include "etcd/discover/service_discovery.h"
#include "service_constants.h"
#include "dao/friend_dao.h"
#include "user_info.h"
#include "user.pb.h"

class FriendService{
public:
    FriendService(FriendDao::Ptr dao, SvcDiscovery::Ptr service_discovery,
        const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
        int timeout_ms, int max_retry
    );

private:
    FriendDao::Ptr dao_;
    SvcDiscovery::Ptr svc_discovery_;
    ::brpc::ChannelOptions options_;

    std::string etcd_server_ip_;
    std::string protocol_;
    int connect_timeout_ms_;
    int timeout_ms_;
    int max_retry_;

    std::vector<UserInfo> GetMultiUserInfo_(std::vector<std::string>& users_id);
    std::vector<UserInfo> GetUserInfo_(const std::string& user_id);
};