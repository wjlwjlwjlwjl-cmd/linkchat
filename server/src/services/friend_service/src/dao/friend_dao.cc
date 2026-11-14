#include "friend_dao.h"
#include "relation_entity.h"

FriendDao::FriendDao(
    const std::string& elastic_url, const std::string& index_name, const std::string& doc_type,
    const char* user, const char* passwd, const char* db_name, const char* host, unsigned int port, int connection_pool_size
){
    db_ = MysqlClient::Create(user, passwd, db_name, host, port, connection_pool_size);

    es_ = std::make_shared<ElasticClient>(index_name, doc_type, std::vector<std::string>{elastic_url});
    ElasticClient::PropType props = {
        {"nickname", {{"type", "text"}, {"analyzer", "ikmax"}}},
        {"email", {{"type", "keyword"}}},
        {"user_id", {{"type", "keyword"}}},
    };
    if(!es_->CreateIndex(props)){
        abort();
    }
}

GetFriendDTO FriendDao::GetFriendList(const std::string& user_id){
    GetFriendDTO dto;
    try{
        auto transaction = MysqlClient::Transaction(db_);
        using query = ::odb::query<::linkchat::RelationEntity>;
        auto results = db_->query<::linkchat::RelationEntity>(query::user_id == user_id);

        std::vector<std::string> friend_ids;
        for(auto& result: results){
            friend_ids.push_back(result.peer_id());
        }

        MysqlClient::Commit(transaction);
        return {{true, ""}, friend_ids};
    }
    catch(const std::exception& e){
        return {{false, e.what()}};
    }
}

CommonDTO FriendDao::FriedRemove(const std::string& user_id, const std::string& peer_id){
    CommonDTO dto;
    try{
        auto transaction = MysqlClient::Transaction(db_);
        using query = ::odb::query<::linkchat::RelationEntity>;
        auto result = db_->query<::linkchat::RelationEntity>(query::user_id == user_id && query::peer_id == peer_id);
        if(result.empty()){
            return {false, "好友关系不存在"};
        }

        long long unsigned count = db_->erase_query<::linkchat::RelationEntity>
            (query::user_id == user_id && query::peer_id == peer_id);

        MysqlClient::Commit(transaction);
        return {true, ""};
    }
    catch(const std::exception& e){
        return {false, e.what()};
    }
}

FriendAddDTO FriendDao::FriendAdd(const std::string& user_id, const std::string& respondent_id){
    try{
        auto transaction = MysqlClient::Transaction(db_);
        std::string event_id = service_prefix::friend_apply_prefix + random_util::GetInstanceId();
        ::linkchat::FriendApply apply(event_id, user_id, respondent_id);
        db_->persist(apply);
        MysqlClient::Commit(transaction);
        return {true, "", event_id};
    }
    catch(const std::exception& e){
        return {false, e.what(), ""};
    }
}

FriendAddProcessDTO FriendDao::FriendAddProcess(const std::string& notify_event_id, bool agree, 
    const std::string& apply_user_id, const std::string& user_id, const std::string& apply_user_name){
    try{
        auto transaction = MysqlClient::Transaction(db_);
        using query = ::odb::query<::linkchat::FriendApply>;
        auto result = db_->query<::linkchat::FriendApply>(query::event_id == notify_event_id);
        if(result.empty()){
            return {false, "好友申请不存在"};
        }

        long long unsigned count = db_->erase_query<::linkchat::FriendApply>
            (query::event_id == notify_event_id);
        MysqlClient::Commit(transaction);
    }
    catch(const std::exception& e){
        return {false, e.what(), ""};
    }

    if(agree){
        try{
            auto transaction = MysqlClient::Transaction(db_);

            // 创建好友关系
            ::linkchat::RelationEntity relation(user_id, apply_user_id);
            db_->persist(relation);

            std::string chat_session_id = service_prefix::chat_session_prefix + random_util::GetInstanceId();

            // 创建单聊会话元信息
            ::linkchat::ChatSession session(chat_session_id, apply_user_name, 
                ::linkchat::ChatSessionType::SINGLE);
            db_->persist(session);

            // 创建单聊会话关系
            ::linkchat::ChatSessionMember member_user(chat_session_id, user_id);
            ::linkchat::ChatSessionMember member_apply_user(chat_session_id, user_id);
            db_->persist(member_user);
            db_->persist(member_apply_user);

            MysqlClient::Commit(transaction);
            return {true, "", chat_session_id};
        }
        catch(const std::exception& e){
            return {false, e.what(), ""};
        }
    }
    return {true, "", ""};
}

GetPendingFriendEventDTO FriendDao::GetPendingFriendEventList(const std::string& user_id){
    try{
        std::vector<std::string> event_ids;
        auto transaction = MysqlClient::Transaction(db_);

        // 查询自己待处理的
        using query = ::odb::query<::linkchat::FriendApply>;
        auto results = db_->query<::linkchat::FriendApply>(query::respondent_id == user_id); 

        for(auto& result: results){
            // 将所有事件 id 都取出来，在服务层拼接上 sender info
            event_ids.push_back(result.event_id());
        }
        MysqlClient::Commit(transaction);
        return {{true, ""}, event_ids};
    }
    catch(const std::exception& e){
        return {{false, e.what()}};
    }
}

FriendSearchDTO FriendDao::FriendSearch(const std::string& search_key, int count){
    std::vector<std::pair<std::string, std::string>> term;
    std::vector<std::pair<std::string, std::string>> matches = {{"nickname", search_key}};
    auto value = es_->SelectTermAndMatch(term, matches); // 这里只做昵称的模糊匹配

    std::vector<std::string> user_ids;
    for(int i = 0; i < count; i++){
        std::string user_id = es_->Id(value, i);
        if(user_id.empty()) break;

        user_ids.push_back(user_id);
    }
    return {true, "", user_ids};
}