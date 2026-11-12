#include "odb_mysql/mysql_client.h"
#include "transmite.pb.h"
#include "domain/user_dto.h"

class TransmiteDao {
public:
    TransmiteDao(const char* user, const char* passwd, const char* db_name = "msg_db", const char* host = "mysql-service", 
        unsigned int port = 3306, int connection_pool_size = 5);

    // 获取发送者信息
    UserDTO GetSenderInfo(const std::string& user_id);
private:
    std::shared_ptr<::odb::database> db_;
};