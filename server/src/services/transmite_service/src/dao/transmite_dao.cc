#include "transmite_dao.h"
#include <exception>

TransmiteDao::TransmiteDao(const char* user, const char* passwd, const char* db_name, const char* host, unsigned int port, int connection_pool_size){
    db_ = ::MysqlClient::Create(user, passwd, db_name, host, port, connection_pool_size);
}

UserDTO TransmiteDao::GetSenderInfo(const std::string& user_id){
    try{
        auto transaction = MysqlClient::Transaction(db_);
        MysqlClient::Commit(transaction);
    }
    catch(const std::exception& e){
        return {e.what(), "", "", "", "", ""};
    }
}
