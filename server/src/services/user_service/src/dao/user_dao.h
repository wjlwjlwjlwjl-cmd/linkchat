#pragma once
#include <odb/core.hxx>
#include <odb/database.hxx>

#include "entity/user_entity.h"
#include "user_entity-odb.hxx"
#include "logging/logger.h"
#include "odb_mysql/mysql_client.h"
#include "random_id.h"
#include "botan/argon2id.h"
#include "domain/user_dto.h"

class UserDao
{
public:
    using Ptr = std::shared_ptr<UserDao>;
    UserDao(const char *user, const char *passwd, const char *db_name, const char *host = "mysql-service",
            unsigned int port = 3306, int connection_pool_size = 5);

    std::string Register(const std::string &nickname, const std::string &password, const std::string &email);

    UserDTO GetUserById(const std::string &user_id);
    UserDTO GetUserByEmail(const std::string &email);
    UserDTO GetUserByNickname(const std::string &nickname);

    std::string SetUserAvatar(const std::string& user_id, const std::string& file_id);
    std::string SetUserNickname(const std::string& user_id, const std::string& nickname);
    std::string SetUserDescription(const std::string& user_id, const std::string& description);
private:
    std::shared_ptr<odb::database> db_;
};
