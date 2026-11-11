#include "user_dao.h"
#include "random_id.h"
#include <odb/exceptions.hxx>

UserDao::UserDao(const char *user, const char *passwd, const char *db_name, const char *host, unsigned int port,
                 int connection_pool_size) {
    db_ = MysqlClient::Create(user, passwd, db_name, host, port, connection_pool_size);
}

std::string UserDao::Register(const std::string &nickname, const std::string &password, const std::string &email){
    try {
        auto transaction = MysqlClient::Transaction(db_);
        std::string user_id = "user" + random_util::GetInstanceId();

        ::linkchat::User user;
        user.set_user_id(user_id);
        
        std::string salted_password = create_password_hash(password);
        user.set_password(salted_password);

        user.set_nickname(nickname); 
        user.set_email(email);
        db_->persist(user);

        MysqlClient::Commit(transaction);
    } 
    catch(const odb::object_already_persistent& e){
        return "user already exists or infomation duplicated";
    }
    catch (const odb::exception &e) {
        LOG_ERROR("Failed to register user {} into db: {}", nickname, e.what());
        return "Failed to register, please check db or tb";
    }
    return "";
}

UserDTO UserDao::GetUserById(const std::string &user_id){
    try{
        auto transaction = MysqlClient::Transaction(db_);

        ::linkchat::User user;
        db_->query_one<::linkchat::User>(odb::query<::linkchat::User>::user_id == user_id, user);

        std::string user_id = user.user_id();
        std::string nickname = user.nickname() ? user.nickname().get(): "";
        std::string email = user.email() ? user.email().get(): "";
        std::string description = user.description() ? user.description().get(): "";
        std::string password = user.password() ? user.password().get(): "";
        std::string avatar_id = user.avatar_id() ? user.avatar_id().get(): "";

        MysqlClient::Commit(transaction);
        return UserDTO {user_id, nickname, description, password, email, avatar_id, true};
    }
    catch(const odb::object_not_persistent& e){
        return UserDTO();
    }
    catch (const odb::exception &e) {
        LOG_ERROR("Failed to get user(user_id) {} from db: {}", user_id, e.what());
        return UserDTO();
    }
    return UserDTO();
}

UserDTO UserDao::GetUserByEmail(const std::string &email){
    try{
        auto transaction = MysqlClient::Transaction(db_);

        ::linkchat::User user;
        db_->query_one<::linkchat::User>(odb::query<::linkchat::User>::email == email, user);

        std::string user_id = user.user_id();
        std::string nickname = user.nickname() ? user.nickname().get(): "";
        std::string email = user.email() ? user.email().get(): "";
        std::string description = user.description() ? user.description().get(): "";
        std::string password = user.password() ? user.password().get(): "";
        std::string avatar_id = user.avatar_id() ? user.avatar_id().get(): "";

        MysqlClient::Commit(transaction);
        return UserDTO {user_id, nickname, description, password, email, avatar_id, true};
    }
    catch(const odb::object_not_persistent& e){
        return UserDTO();
    }
    catch (const odb::exception &e) {
        LOG_ERROR("Failed to get user(email) {} from db: {}", email, e.what());
        return UserDTO();
    }
    return UserDTO();
}

UserDTO UserDao::GetUserByNickname(const std::string &nickname){
    try{
        auto transaction = MysqlClient::Transaction(db_);

        ::linkchat::User user;
        db_->query_one<::linkchat::User>(odb::query<::linkchat::User>::nickname == nickname, user);

        std::string user_id = user.user_id();
        std::string nickname = user.nickname() ? user.nickname().get(): "";
        std::string email = user.email() ? user.email().get(): "";
        std::string description = user.description() ? user.description().get(): "";
        std::string password = user.password() ? user.password().get(): "";
        std::string avatar_id = user.avatar_id() ? user.avatar_id().get(): "";

        MysqlClient::Commit(transaction);
        return UserDTO {user_id, nickname, description, password, email, avatar_id, true};
    }
    catch(const odb::object_not_persistent& e){
        return UserDTO();
    }
    catch (const odb::exception &e) {
        LOG_ERROR("Failed to get user(nickname) {} from db: {}", nickname, e.what());
        return UserDTO();
    }
    return UserDTO();
}

std::string UserDao::SetUserAvatar(const std::string& user_id, const std::string& file_id){
    try{
        auto transaction = MysqlClient::Transaction(db_);

        ::linkchat::User user;
        db_->query_one<::linkchat::User>(odb::query<::linkchat::User>::user_id == user_id, user);

        user.set_avatar_id(file_id);
        db_->persist(user);

        MysqlClient::Commit(transaction);
    }
    catch(const odb::object_not_persistent& e){
        return "User not found";
    }
    catch (const odb::exception &e) {
        LOG_ERROR("Failed to set user avatar {} into db: {}", user_id, e.what());
        return "Failed to set user avatar, please check db or tb";
    }
    return "";
}

std::string UserDao::SetUserNickname(const std::string& user_id, const std::string& nickname){
    try{
        auto transaction = MysqlClient::Transaction(db_);

        ::linkchat::User user;
        db_->query_one<::linkchat::User>(odb::query<::linkchat::User>::user_id == user_id, user);

        user.set_nickname(nickname);
        db_->persist(user);

        MysqlClient::Commit(transaction);
    }
    catch(const odb::object_not_persistent& e){
        return "User not found";
    }
    catch (const odb::exception &e) {
        LOG_ERROR("Failed to set user avatar {} into db: {}", user_id, e.what());
        return "Failed to set user avatar, please check db or tb";
    }
    return "";
}

std::string UserDao::SetUserDescription(const std::string& user_id, const std::string& description){
    try{
        auto transaction = MysqlClient::Transaction(db_);

        ::linkchat::User user;
        db_->query_one<::linkchat::User>(odb::query<::linkchat::User>::user_id == user_id, user);

        user.set_description(description);
        db_->persist(user);

        MysqlClient::Commit(transaction);
    }
    catch(const odb::object_not_persistent& e){
        return "User not found";
    }
    catch (const odb::exception &e) {
        LOG_ERROR("Failed to set user avatar {} into db: {}", user_id, e.what());
        return "Failed to set user avatar, please check db or tb";
    }
    return "";

}