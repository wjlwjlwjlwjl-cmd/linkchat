#include "transmite_dao.h"
#include <exception>

TransmiteDao::TransmiteDao(const char* user, const char* passwd, const char* db_name, const char* host, unsigned int port, int connection_pool_size){
    db_ = ::MysqlClient::Create(user, passwd, db_name, host, port, connection_pool_size);
}

UserDTO TransmiteDao::GetSenderInfo(const std::string& user_id){
    try{
        auto transaction = MysqlClient::Transaction(db_);
        using query = odb::query<::linkchat::User>;
        auto user = db_->query_one<::linkchat::User>(query::user_id == user_id);
        if(!user){
            return {"User not found", "", "", "", "", ""};
        }

        std::string nickname, description, email, avatar;
        if(user->has_nickname()) nickname = user->nickname().get();
        if(user->has_description()) description = user->description().get();
        if(user->has_email()) email = user->email().get();
        if(user->has_avatar_id()) avatar = user->avatar_id().get();
        MysqlClient::Commit(transaction);
        return {
            "", 
            user->user_id(),
            nickname,
            description,
            email,
            avatar,
        };
    }
    catch(const std::exception& e){
        return {e.what(), "", "", "", "", ""};
    }
    return {};
}

std::vector<std::string> TransmiteDao::GetChatSessionUsers(const std::string& chat_session_id){

}