// 好友数据访问层

#pragma once

#include "domain/get_friend_dto.h"
#include "odb_mysql/mysql_client.h"
#include "relation.h"
#include "relation-odb.hxx"

class FriendDao {
public:
    FriendDao(const char* user, const char* passwd, const char* db_name = "linkchat", 
        const char* host = "mysql-service", 
        unsigned int port = 3306, int connection_pool_size = 5);

    GetFriendDTO GetFriendList(const std::string& user_id);
private:
    MysqlClient::Ptr db_;
};