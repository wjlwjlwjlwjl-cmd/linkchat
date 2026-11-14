#include "friend_dao.h"

FriendDao::FriendDao(const char* user, const char* passwd, const char* db_name, 
    const char* host, 
    unsigned int port, int connection_pool_size){

}

GetFriendDTO FriendDao::GetFriendList(const std::string& user_id){
    GetFriendDTO dto;
    try{
        auto transaction = MysqlClient::Transaction(db_);
        
        MysqlClient::Commit(transaction);
    }
    catch(const std::exception& e){

    }
    return dto;
}