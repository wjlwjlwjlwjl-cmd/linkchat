#include "friend_manager.h"
#include "base.pb.h"
#include "friend.pb.h"
#include "user.pb.h"

FriendManager::FriendManager(FriendDao::Ptr dao, SvcDiscovery::Ptr service_discovery,
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

::linkchat::GetFriendListRsp FriendManager::GetFriendList(const std::string& user_id){
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
    if(friends_info.size() != friends_id.size()) 
        LOG_ERROR("INFO of some users failed to select, {}, {}", friends_info.size(), friends_id.size());

    std::vector<::linkchat::UserInfo> infos;
    std::transform(friends_info.begin(), friends_info.end(),
        std::back_inserter(infos), [&](const auto& kv){
        return kv.second;
    });
    response.mutable_friend_list()->Add(infos.begin(), infos.end());

    response.set_success(true);
    return response;
}

::linkchat::FriendRemoveRsp FriendManager::FriendRemove(const std::string& user_id, const std::string& peer_id){
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

::linkchat::FriendAddRsp FriendManager::FriendAdd(const std::string& user_id, const std::string& respondent_id){
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

::linkchat::FriendAddProcessRsp FriendManager::FriendAddProcess(const std::string& notify_event_id, bool agree, 
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

::linkchat::GetPendingFriendEventListRsp FriendManager::GetPendingFriendEventList(const std::string& user_id){
    ::linkchat::GetPendingFriendEventListRsp response;
    auto dto = dao_->GetPendingFriendEventList(user_id);
    if(!dto.success){
        LOG_ERROR("get pending friend event list failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("get pending friend event list failed");
        return response;
    }
    auto ev_resp_list = dto.ev_resp_list;

    std::vector<std::string> respondents_id;
    for(auto& [event_id, respondent_id]: ev_resp_list){
        respondents_id.push_back(respondent_id);
    }

    auto respondents_info = GetMultiUserInfo_(respondents_id);
    if(respondents_info.size() != respondents_id.size()) 
        LOG_ERROR("INFO of some users failed to select, {}, {}", respondents_info.size(), respondents_id.size());

    std::vector<::linkchat::FriendEvent> events;
    std::transform(respondents_info.begin(), respondents_info.end(),
        std::back_inserter(events), [&](const auto& kv){
            ::linkchat::FriendEvent event;
            event.set_event_id(kv.first);
            event.mutable_sender()->CopyFrom(kv.second);
            return event;
    });

    response.mutable_event()->Add(events.begin(), events.end());
    response.set_success(true);
    return response;
}

::linkchat::FriendSearchRsp FriendManager::FriendSearch(const std::string& search_key, int count){
    ::linkchat::FriendSearchRsp response;
    auto dto = dao_->FriendSearch(search_key, count);
    if(!dto.success){
        LOG_ERROR("friend search failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("friend search failed");
        return response;
    }

    std::vector<std::string> users_id = dto.users_id;
    std::vector<::linkchat::UserInfo> user_infos;
    for(auto& user_id: users_id){
        auto user_info = GetUserInfo_(user_id);
        if(!user_info.has_value()){
            LOG_ERROR("user not found, user_id: {}", user_id);
            continue;
        }
        user_infos.push_back(user_info.value());
    }

    response.set_success(true);
    response.mutable_user_info()->Add(user_infos.begin(), user_infos.end());

    return response;
}

::linkchat::GetChatSessionListRsp FriendManager::GetChatSessionList(const std::string& user_id){
    ::linkchat::GetChatSessionListRsp response;

    auto dto = dao_->GetChatSession(user_id);
    if(!dto.success){
        LOG_ERROR("get chat session list failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("get chat session list failed");
        return response;
    }

    std::vector<::linkchat::ChatSessionInfo> infos;
    std::transform(dto.chat_sessions.begin(), dto.chat_sessions.end(),
        std::back_inserter(infos), [&](const auto& chat_session){
            ::linkchat::ChatSessionInfo info;

            if(chat_session.single_chat_friend_id.has_value()){
                info.set_single_chat_friend_id(chat_session.single_chat_friend_id.value());
            }
            info.set_chat_session_id(chat_session.chat_session_id);
            info.set_chat_session_name(chat_session.chat_session_name);
            info.set_avatar(chat_session.avatar);

            if(chat_session.prev_message.has_value()){
                ::linkchat::MessageInfo prev_message;

                ::linkchat::UserInfo sender;
                sender.set_user_id(chat_session.prev_message.value().sender.user_id);
                sender.set_nickname(chat_session.prev_message.value().sender.nickname);
                sender.set_description(chat_session.prev_message.value().sender.description);
                sender.set_email(chat_session.prev_message.value().sender.email);
                sender.set_avatar(chat_session.prev_message.value().sender.avatar);
                prev_message.mutable_sender()->CopyFrom(sender);

                ::linkchat::MessageContent message_content;
                if(chat_session.prev_message.value().message.message_type == MessageType::STRING){
                    message_content.set_message_type(::linkchat::MessageType::STRING);
                    message_content.mutable_string_message()->set_content(chat_session.prev_message.value().message.string_message->content);
                }else{
                    message_content.set_message_type(::linkchat::MessageType::FILE);

                    auto file_msg = chat_session.prev_message.value().message.file_message.value();
                    ::linkchat::FileMessageInfo file_message;
                    file_message.set_file_id(file_msg.file_id.value());
                    file_message.set_file_name(file_msg.file_name.value());
                    file_message.set_file_size(file_msg.file_size.value());
                    message_content.mutable_file_message()->CopyFrom(file_message);
                }
                prev_message.mutable_message()->CopyFrom(message_content);

                prev_message.set_message_id(chat_session.prev_message.value().message_id);
                prev_message.set_chat_session_id(chat_session.prev_message.value().chat_session_id);

                info.mutable_prev_message()->CopyFrom(prev_message);
            }
            return info;
    });
    response.mutable_chat_session_info_list()->Add(infos.begin(), infos.end());

    response.set_success(true);
    return response;
}

::linkchat::ChatSessionCreateRsp FriendManager::ChatSessionCreate(const std::string& user_id, const std::string& chat_session_name, 
    std::vector<std::string>& member_id_list){
    ::linkchat::ChatSessionCreateRsp response;

    auto dto = dao_->CreateChatSession(user_id, chat_session_name, member_id_list);
    if(!dto.success){
        LOG_ERROR("create chat session failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("create chat session failed");
        return response;
    }
    response.set_success(true);
    response.set_chat_session_id(dto.chat_session_id);
    return response;
}

::linkchat::GetChatSessionMemberRsp FriendManager::GetChatSessionMember(const std::string& user_id, const std::string& chat_session_id){
    ::linkchat::GetChatSessionMemberRsp response;
    auto dto = dao_->GetChatSessionMember(user_id, chat_session_id);
    if(!dto.success){
        LOG_ERROR("get chat session member failed: ", dto.errmsg);
        response.set_success(false);
        response.set_errmsg("get chat session member failed");
        return response;
    }

    // 在这里获取 user_info 信息
    auto user_infos = GetMultiUserInfo_(dto.members_id);
    std::vector<::linkchat::UserInfo> member_infos;
    for(auto& e: user_infos){
        member_infos.push_back(e.second);
    }
    response.mutable_member_info_list()->Add(member_infos.begin(), member_infos.end());
    response.set_success(true);

    return response;
}

//////////////////////////////////////////
/// 私有方法
//////////////////////////////////////////
std::unordered_map<std::string, ::linkchat::UserInfo> 
    FriendManager::GetMultiUserInfo_(std::vector<std::string>& users_id){

    std::string ip;
    auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
    if(ip.empty()){
        LOG_ERROR("No nodes available for {}", service_name::user_service);
        return {};
    }
    if(channel->Init(ip.c_str(), &options_)){
        LOG_ERROR("Init channel failed, ip: {}", ip);
        return {};
    }

    std::unordered_map<std::string, ::linkchat::UserInfo> ret;

    ::brpc::Controller controller;
    ::linkchat::UserService_Stub stub(channel.get());
    ::linkchat::GetMultiUserInfoReq request;
    ::linkchat::GetMultiUserInfoRsp response;
    request.mutable_users_id()->Add(users_id.begin(), users_id.end());

    stub.GetMultiUserInfo(&controller, &request, &response, nullptr);

    if(controller.Failed()){
        LOG_ERROR("Failed to call user_service: ", controller.ErrorText());
        return {};
    }
    if(!response.success()){ // 实际上 user_service 会尽可能返回查询出来的 UserInfo，没查出来的自动忽略，这里总会是 true
        LOG_ERROR("Call user_service fail: ", response.errmsg());
        return {};
    }
    for(auto& e: response.users_info()){
        ret[e.first] = e.second;
    }
    return ret;
}

std::optional<::linkchat::UserInfo> FriendManager::GetUserInfo_(const std::string& user_id){
    std::string ip;
    auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
    if(ip.empty()){
        LOG_ERROR("No nodes available for {}", service_name::user_service);
        return {};
    }
    if(channel->Init(ip.c_str(), &options_)){
        LOG_ERROR("Init channel failed, ip: {}", ip);
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