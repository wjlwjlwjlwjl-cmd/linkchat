#include "friend_dao.h"
#include "domain/get_chat_session_dto.h"
#include "relation_entity.h"
#include "message_entity.h"
#include "message_entity-odb.hxx"
#include <odb/query.hxx>

namespace {

constexpr signed char kMsgTypeString = 0;
constexpr signed char kMsgTypeFile = 1;

// 填充会话的上一条消息，发送者的昵称、头像等信息由 service 层补全
void FillPrevMessage(odb::database& db, ChatSessionInfo& info) {
    using query = odb::query<MessageEntity>;
    auto rows = db.query<MessageEntity>(
        (query::to_session_id == info.chat_session_id)
        + " ORDER BY " + query::create_time + " DESC LIMIT 1");

    for (auto& msg : rows) {
        MessageInfo prev_message;
        prev_message.message_id = msg.message_id();
        prev_message.chat_session_id = msg.to_session_id();
        prev_message.sender.user_id = msg.from_user_id();

        if (msg.message_type() == kMsgTypeString) {
            // 文本消息
            prev_message.message.message_type = MessageType::STRING;
            if (msg.has_content()) {
                StringMessageInfo string_message;
                string_message.content = msg.content().get();
                prev_message.message.string_message = std::move(string_message);
            }
        }
        else if (msg.message_type() == kMsgTypeFile) {
            // 文件消息
            prev_message.message.message_type = MessageType::FILE;
            FileMessageInfo file_message;
            if (msg.has_file_id()) {
                file_message.file_id = msg.file_id().get();
            }
            if (msg.has_file_name()) {
                file_message.file_name = msg.file_name().get();
            }
            if (msg.has_file_size()) {
                file_message.file_size = static_cast<int64_t>(msg.file_size().get());
            }
            prev_message.message.file_message = std::move(file_message);
        }
        else {
            break;
        }

        info.prev_message = std::move(prev_message);
        break;
    }
}

} // namespace

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

            std::string chat_session_id = service_prefix::chat_session_single_prefix + random_util::GetInstanceId();

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

GetChatSessionDTO FriendDao::GetChatSession(const std::string& user_id){
    GetChatSessionDTO dto;
    try{
        auto transaction = MysqlClient::Transaction(db_);

        // 单聊
        using single_query = odb::query<::linkchat::SingleChatSession>;
        auto single_rows = db_->query<::linkchat::SingleChatSession>(
            single_query::css::chat_session_type == ::linkchat::ChatSessionType::SINGLE &&
            single_query::csm1::user_id == user_id &&
            single_query::csm2::user_id != single_query::csm1::user_id); //当前用户，且非自己

        for(const auto& row : single_rows){
            ChatSessionInfo info;
            info.chat_session_id = row.chat_session_id;
            info.single_chat_friend_id = row.friend_id;
            // 单聊名称/头像需后续通过 user 服务补全
            FillPrevMessage(*db_, info);
            dto.chat_sessions.push_back(std::move(info));
        }

        // 群聊
        using group_query = odb::query<::linkchat::GroupChatSession>;
        auto group_rows = db_->query<::linkchat::GroupChatSession>(
            group_query::css::chat_session_type == ::linkchat::ChatSessionType::GROUP &&
            group_query::csm::user_id == user_id);

        for(const auto& row : group_rows){
            ChatSessionInfo info;
            info.chat_session_id = row.chat_session_id;
            info.chat_session_name = row.chat_session_name;
            FillPrevMessage(*db_, info);
            dto.chat_sessions.push_back(std::move(info));
        }

        MysqlClient::Commit(transaction);
        dto.success = true;
        return dto;
    }
    catch(const std::exception& e){
        dto.success = false;
        dto.errmsg = e.what();
        return dto;
    }
}

ChatSessionCreateDTO FriendDao::CreateChatSession(const std::string& user_id, const std::string& chat_session_name, 
    const std::vector<std::string>& member_id_list){
    try{
        auto transaction = MysqlClient::Transaction(db_);

        // 先创建群聊的元信息
        ::linkchat::ChatSession session;
        std::string session_id = service_prefix::chat_session_group_prefix + random_util::GetInstanceId();
        session.set_chat_session_id(session_id);
        session.set_chat_session_name(chat_session_name);
        session.set_chat_session_type(::linkchat::ChatSessionType::GROUP);
        db_->persist(session);

        // 加下来往群里加人
        for(auto& mem_id: member_id_list){
            ::linkchat::ChatSessionMember member(session_id, mem_id);
            db_->persist(member);
        }

        MysqlClient::Commit(transaction);
        return {true, "", session_id, chat_session_name};
    }
    catch(const std::exception& e){
        return {false, e.what()};
    }
}

GetChatSessionMemberDTO FriendDao::GetChatSessionMember(const std::string& user_id, const std::string& chat_session_id){
    std::vector<std::string> users_id;
    try{
        auto transaction = MysqlClient::Transaction(db_);
        // 查询群聊成员
        using query = ::odb::query<::linkchat::ChatSessionMember>;
        auto results = db_->query<::linkchat::ChatSessionMember>(
            query::chat_session_id == chat_session_id
        );
        for(auto& result: results){
            users_id.push_back(result.user_id());
        }

        MysqlClient::Commit(transaction);
        return {true, "", users_id};
    }
    catch(const std::exception& e){
        return {false, e.what()};
    }
}