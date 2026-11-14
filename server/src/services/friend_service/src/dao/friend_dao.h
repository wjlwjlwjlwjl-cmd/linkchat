// 好友数据访问层

#pragma once
#include "random_id.h"
#include "service_constants.h"

#include "common_dto.h"
#include "domain/get_friend_dto.h"
#include "domain/friend_add_dto.h"
#include "domain/friend_add_process_dto.h"
#include "domain/get_pending_friend_event_dto.h"
#include "domain/friend_search_dto.h"
#include "domain/get_chat_session_dto.h"
#include "domain/chat_session_create_dto.h"
#include "domain/get_chat_session_member.h"

#include "elastic/elastic_client.h"

#include "odb_mysql/mysql_client.h"
#include "relation_entity.h"
#include "relation_entity-odb.hxx"
#include "friend_entity.h"
#include "friend_entity-odb.hxx"
#include "chat_session.h"
#include "chat_session-odb.hxx"
#include "chat_session_member.h"
#include "chat_session_member-odb.hxx"

class FriendDao {
public:
    using Ptr = std::shared_ptr<FriendDao>;

    FriendDao(const std::string& elastic_url, const std::string& index_name, const std::string& doc_type,
        const char* user, const char* passwd, const char* db_name = "linkchat", 
        const char* host = "mysql-service", unsigned int port = 3306, int connection_pool_size = 5
    );

    /// @brief 获取用户好友列表
    /// @param user_id 用户ID
    /// @return GetFriendDTO 好友列表DTO
    GetFriendDTO GetFriendList(const std::string& user_id);

    /// @brief 删除好友关系
    /// @param user_id 用户ID
    /// @param peer_id 好友ID
    /// @return CommonDTO 操作结果DTO
    CommonDTO FriedRemove(const std::string& user_id, const std::string& peer_id);

    /// @brief 发起好友申请，会返回通知id，网关推送给客户端，客户端处理好友申请时会携带通知id给 FriendAddProcess
    /// @param user_id 申请人 ID
    /// @param respondent_id 被申请人 ID
    /// @return FriendAddDTO 好友添加DTO
    FriendAddDTO FriendAdd(const std::string& user_id, const std::string& respondent_id);

    /// @brief 好友申请的处理
    /// @param notify_event_id 通知事件ID
    /// @param agree 是否同意好友申请
    /// @param apply_user_id 申请人的用户ID
    /// @param user_id 被申请人用户ID
    /// @return FriendAddProcessDTO 好友申请处理DTO
    FriendAddProcessDTO FriendAddProcess(const std::string& notify_event_id, bool agree, 
        const std::string& apply_user_id, const std::string& user_id, const std::string& apply_user_name);

    /// @brief 获取自己待处理的好友申请事件
    /// @param user_id 用户ID
    /// @return GetPendingFriendEventDTO 待处理好友申请列表DTO
    GetPendingFriendEventDTO GetPendingFriendEventList(const std::string& user_id);

    /// @brief 搜索好友
    /// @param search_key 搜索关键词
    /// @param user_id 用户ID
    /// @return FriendSearchDTO 好友搜索DTO
    FriendSearchDTO FriendSearch(const std::string& search_key, int count);

    /// @brief 获取用户聊天会话列表，这里只是填充了聊天相关信息和上一条消息信息，但是会话头像需要后续通过 user 服务补全（service层完成）
    /// @param user_id 用户ID
    /// @return GetChatSessionDTO 聊天会话列表DTO
    GetChatSessionDTO GetChatSession(const std::string& user_id);

    /// @brief 创建群聊会话
    /// @param chat_session_create_dto 创建会话DTO
    /// @return ChatSessionCreateDTO 创建会话DTO
    ChatSessionCreateDTO CreateChatSession(const std::string& user_id, const std::string& chat_session_name, 
        const std::vector<std::string>& member_id_list);

    /// @brief 获取群聊会话成员列表
    /// @param get_chat_session_member_dto 获取会话成员列表DTO
    /// @return GetChatSessionMemberDTO 获取会话成员列表DTO
    GetChatSessionMemberDTO GetChatSessionMember(const std::string& user_id, const std::string& chat_session_id);
private:
    MysqlClient::Ptr db_;
    ElasticClient::Ptr es_;
};
