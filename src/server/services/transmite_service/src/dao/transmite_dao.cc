#include "transmite_dao.h"
#include "error_msg.h"
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
            return {"User not found", {}};
        }

        UserInfo info;
        info.user_id = user->user_id();
        if(user->has_nickname()) info.nickname = user->nickname().get();
        if(user->has_description()) info.description = user->description().get();
        if(user->has_email()) info.email = user->email().get();
        if(user->has_avatar_id()) info.avatar = user->avatar_id().get();
        MysqlClient::Commit(transaction);
        return {"", info};
    }
    catch(const std::exception& e){
        return {e.what(), {}};
    }
    return {};
}

// todo
std::vector<std::string> TransmiteDao::GetChatSessionUsers(const std::string& chat_session_id){
    return std::vector<std::string>();
}

std::string TransmiteDao::ValidateSender(const std::string& user_id, const std::string& chat_session_id){
    try{
        auto transaction = MysqlClient::Transaction(db_);

        using session_query = odb::query<::linkchat::ChatSession>;
        auto session = db_->query_one<::linkchat::ChatSession>(
            session_query::chat_session_id == chat_session_id);
        if(!session){
            return errmsg::transmite::kChatSessionNotFound;
        }

        using member_query = odb::query<::linkchat::ChatSessionMember>;

        if(session->chat_session_type() == ::linkchat::ChatSessionType::SINGLE){
            // 单聊：对方是该会话中除发送者外的另一个成员
            auto peer = db_->query_one<::linkchat::ChatSessionMember>(
                member_query::chat_session_id == chat_session_id &&
                member_query::user_id != user_id);
            if(!peer){
                return errmsg::transmite::kChatSessionNotFound;
            }

            // 单聊需要好友关系：删除好友会双向移除关系，
            // 被删方这里的关系也随之消失，发送时提示重新添加好友
            using relation_query = odb::query<::linkchat::RelationEntity>;
            auto relation = db_->query_one<::linkchat::RelationEntity>(
                relation_query::user_id == user_id &&
                relation_query::peer_id == peer->user_id());
            MysqlClient::Commit(transaction);
            if(!relation){
                return errmsg::transmite::kNotFriend;
            }
            return "";
        }

        // 群聊：发送者必须是该会话的成员
        auto member = db_->query_one<::linkchat::ChatSessionMember>(
            member_query::chat_session_id == chat_session_id &&
            member_query::user_id == user_id);
        MysqlClient::Commit(transaction);
        if(!member || member->is_del()){
            return errmsg::transmite::kNotGroupMember;
        }
        return "";
    }
    catch(const std::exception& e){
        return e.what();
    }
}