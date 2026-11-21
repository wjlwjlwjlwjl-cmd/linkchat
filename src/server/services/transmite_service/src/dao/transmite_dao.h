#pragma once

#include "odb_mysql/mysql_client.h"
#include "domain/user_dto.h"
#include "user_entity.h"
#include "user_entity-odb.hxx"
#include "relation_entity.h"
#include "relation_entity-odb.hxx"
#include "chat_session.h"
#include "chat_session-odb.hxx"
#include "chat_session_member.h"
#include "chat_session_member-odb.hxx"

class TransmiteDao {
public:
    using Ptr = std::shared_ptr<TransmiteDao>;
    TransmiteDao(const char* user, const char* passwd, const char* db_name = "linkchat", const char* host = "mysql-service", 
        unsigned int port = 3306, int connection_pool_size = 5);

    // 获取发送者信息
    UserDTO GetSenderInfo(const std::string& user_id);

    // 获取一个会话的全部用户
    std::vector<std::string> GetChatSessionUsers(const std::string& chat_session_id);

    // 校验用户能否在该会话发消息：单聊校验好友关系，群聊校验成员身份；返回空串表示校验通过
    std::string ValidateSender(const std::string& user_id, const std::string& chat_session_id);
private:
    std::shared_ptr<::odb::database> db_;
};