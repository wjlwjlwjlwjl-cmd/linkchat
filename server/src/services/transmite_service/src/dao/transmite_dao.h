#pragma once

#include "odb_mysql/mysql_client.h"
#include "transmite.pb.h"
#include "domain/user_dto.h"
#include "user_entity.h"
#include "user_entity-odb.hxx"

class TransmiteDao {
public:
    using Ptr = std::shared_ptr<TransmiteDao>;
    TransmiteDao(const char* user, const char* passwd, const char* db_name = "linkchat", const char* host = "mysql-service", 
        unsigned int port = 3306, int connection_pool_size = 5);

    // 获取发送者信息
    UserDTO GetSenderInfo(const std::string& user_id);

    // 获取一个会话的全部用户
    std::vector<std::string> GetChatSessionUsers(const std::string& chat_session_id);
private:
    std::shared_ptr<::odb::database> db_;
};