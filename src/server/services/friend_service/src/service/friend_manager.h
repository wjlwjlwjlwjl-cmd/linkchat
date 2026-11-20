#pragma once

#include <brpc/server.h>

#include "etcd/discover/service_discovery.h"
#include "service_constants.h"
#include "dao/friend_dao.h"
#include "user_info.h"
#include "user.pb.h"
#include "friend.pb.h"

class FriendManager{
public:
    using Ptr = std::shared_ptr<FriendManager>;

    FriendManager(FriendDao::Ptr dao, SvcDiscovery::Ptr service_discovery,
        const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
        int timeout_ms, int max_retry
    );

    ::linkchat::GetFriendListRsp GetFriendList(const std::string& user_id);

    ::linkchat::FriendRemoveRsp FriendRemove(const std::string& user_id, const std::string& peer_id);
    ::linkchat::FriendAddRsp FriendAdd(const std::string& user_id, const std::string& respondent_id);
    ::linkchat::FriendAddProcessRsp FriendAddProcess(const std::string& notify_event_id, bool agree, 
        const std::string& apply_user_id, const std::string& user_id);
    ::linkchat::GetPendingFriendEventListRsp GetPendingFriendEventList(const std::string& user_id);

    ::linkchat::FriendSearchRsp FriendSearch(const std::string& search_key, int count);

    ::linkchat::GetChatSessionListRsp GetChatSessionList(const std::string& user_id);
    ::linkchat::ChatSessionCreateRsp ChatSessionCreate(const std::string& user_id, const std::string& chat_session_name, 
        std::vector<std::string>& member_id_list);
    ::linkchat::GetChatSessionMemberRsp GetChatSessionMember(const std::string& user_id, const std::string& chat_session_id);
    ::linkchat::GetSingleChatSessionRsp GetSingleChatSession(const std::string& user_id, const std::string& friend_id);

private:
    FriendDao::Ptr dao_;
    SvcDiscovery::Ptr svc_discovery_;
    ::brpc::ChannelOptions options_;

    std::string etcd_server_ip_;
    std::string protocol_;
    int connect_timeout_ms_;
    int timeout_ms_;
    int max_retry_;

    // user_id，user_info
    std::unordered_map<std::string, linkchat::UserInfo> GetMultiUserInfo_(std::vector<std::string>& users_id);
    std::optional<::linkchat::UserInfo> GetUserInfo_(const std::string& user_id);
};