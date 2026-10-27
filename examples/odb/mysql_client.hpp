#pragma once

#include "person.hxx"
#include "person-odb.hxx"

#include <odb/mysql/connection-factory.hxx>
#include <odb/mysql/database.hxx>

class MysqlClient{
public:
    /// @brief 初始化数据库连接
    /// @param user 用户名
    /// @param passwd 密码
    /// @param db_name 数据库名称
    /// @param host server addr
    /// @param port 端口
    /// @param connection_pool_size 连接池大小
    /// @return 数据库操作句柄
    static std::shared_ptr<odb::database> Create(const char* user,
                const char* passwd,
                const char* db_name,
                const char* host = "mysql-service",
                unsigned int port = 3306, 
                int connection_pool_size = 5){
        std::unique_ptr<odb::mysql::connection_factory> pool(new odb::mysql::connection_pool_factory(connection_pool_size));
        std::shared_ptr<odb::database> db(new odb::mysql::database(user, passwd, 
            db_name, host, port, 0, "utf8", 0, std::move(pool)));
        return db;
    }

    /// @brief 创建并开启事务
    /// @param db 数据库操作句柄
    /// @return 事务句柄
    static std::shared_ptr<odb::transaction> Transaction(std::shared_ptr<odb::database>& db){
        return std::make_shared<odb::transaction>(db->begin());
    }

    /// @brief 提交事务
    /// @param ta 事务句柄
    static void Commit(std::shared_ptr<odb::transaction>& ta){
        ta->commit();
    }

    /// @brief 回滚事务
    /// @param ta 事务句柄
    static void Rollback(std::shared_ptr<odb::transaction>& ta){
        ta->rollback();
    }
};