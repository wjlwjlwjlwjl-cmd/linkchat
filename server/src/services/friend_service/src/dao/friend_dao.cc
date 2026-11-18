#include "friend_dao.h"
#include "domain/get_chat_session_dto.h"
#include "relation_entity.h"
#include "message_entity.h"
#include "message_entity-odb.hxx"
#include <odb/query.hxx>
#include <set>

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

CommonDTO FriendDao::FriendRemove(const std::string& user_id, const std::string& peer_id){
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

        // 删除时，将 user_id 与 peer_id 的单聊会话中，user_id 自己的成员记录的 is_del_ 置为 1
        using single_query = ::odb::query<::linkchat::SingleChatSession>;
        auto single_session = db_->query_one<::linkchat::SingleChatSession>(
            single_query::css::chat_session_type == ::linkchat::ChatSessionType::SINGLE &&
            single_query::csm1::user_id == user_id &&
            single_query::csm2::user_id == peer_id);

        if(single_session){
            std::string chat_session_id = single_session->chat_session_id;

            using member_query = ::odb::query<::linkchat::ChatSessionMember>;
            auto chat_session_member = db_->query_one<::linkchat::ChatSessionMember>(
                member_query::chat_session_id == chat_session_id &&
                member_query::user_id == user_id
            );
            if(chat_session_member){
                chat_session_member->set_is_del(true);
                db_->update(*chat_session_member);
            }
        }

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

        auto respondent = db_->query_one<::linkchat::User>(::odb::query<::linkchat::User>::user_id == respondent_id);
        if(!respondent){
            return {false, "好友不存在", ""};
        }

        // 已经是好友关系，直接失败返回
        using relation_query = ::odb::query<::linkchat::RelationEntity>;
        auto relation = db_->query_one<::linkchat::RelationEntity>(
            relation_query::user_id == user_id && relation_query::peer_id == respondent_id);
        if(relation){
            return {false, "已经是好友关系", ""};
        }

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
    const std::string& apply_user_id, const std::string& respondent_id){
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

            // 创建好友关系，好友关系是双向的（已存在的方向不重复插入）
            using relation_query = ::odb::query<::linkchat::RelationEntity>;
            auto relation1 = db_->query_one<::linkchat::RelationEntity>(
                relation_query::user_id == respondent_id && relation_query::peer_id == apply_user_id);
            if(!relation1){
                ::linkchat::RelationEntity new_relation1(respondent_id, apply_user_id);
                db_->persist(new_relation1);
            }
            auto relation2 = db_->query_one<::linkchat::RelationEntity>(
                relation_query::user_id == apply_user_id && relation_query::peer_id == respondent_id);
            if(!relation2){
                ::linkchat::RelationEntity new_relation2(apply_user_id, respondent_id);
                db_->persist(new_relation2);
            }

            // 查询双方是否已经存在单聊会话（不区分 is_del_，删除后重新添加需要复用原会话）
            using member_query = ::odb::query<::linkchat::ChatSessionMember>;
            std::set<std::string> apply_user_sessions;
            {
                auto apply_members = db_->query<::linkchat::ChatSessionMember>(
                    member_query::user_id == apply_user_id);
                for(auto& member: apply_members){
                    apply_user_sessions.insert(member.chat_session_id());
                }
            }

            // 先收集双方共同的会话 id，避免在遍历结果时嵌套查询
            std::vector<std::string> common_session_ids;
            {
                auto respondent_members = db_->query<::linkchat::ChatSessionMember>(
                    member_query::user_id == respondent_id);
                for(auto& member: respondent_members){
                    if(apply_user_sessions.count(member.chat_session_id()) != 0){
                        common_session_ids.push_back(member.chat_session_id());
                    }
                }
            }

            // 只复用单聊会话，群聊不算
            using session_query = ::odb::query<::linkchat::ChatSession>;
            std::string chat_session_id;
            for(auto& session_id: common_session_ids){
                auto session = db_->query_one<::linkchat::ChatSession>(
                    session_query::chat_session_id == session_id);
                if(session && session->chat_session_type() == ::linkchat::ChatSessionType::SINGLE){
                    chat_session_id = session_id;
                    break;
                }
            }

            if(!chat_session_id.empty()){
                // 会话已存在，只将双方成员记录的 is_del_ 置为 0，不新增记录
                auto member_respondent_user = db_->query_one<::linkchat::ChatSessionMember>(
                    member_query::chat_session_id == chat_session_id &&
                    member_query::user_id == respondent_id);
                if(member_respondent_user && member_respondent_user->is_del()){
                    member_respondent_user->set_is_del(false);
                    db_->update(*member_respondent_user);
                }

                auto member_apply_user = db_->query_one<::linkchat::ChatSessionMember>(
                    member_query::chat_session_id == chat_session_id &&
                    member_query::user_id == apply_user_id);
                if(member_apply_user && member_apply_user->is_del()){
                    member_apply_user->set_is_del(false);
                    db_->update(*member_apply_user);
                }
            }
            else{
                chat_session_id = service_prefix::chat_session_single_prefix + random_util::GetInstanceId();

                // 创建单聊会话元信息，单聊会话不需要会话名称
                ::linkchat::ChatSession session;
                session.set_chat_session_id(chat_session_id);
                session.set_chat_session_type(::linkchat::ChatSessionType::SINGLE);
                db_->persist(session);

                // 创建单聊会话关系
                ::linkchat::ChatSessionMember member_respondent_user(chat_session_id, respondent_id);
                ::linkchat::ChatSessionMember member_apply_user(chat_session_id, apply_user_id);
                db_->persist(member_respondent_user);
                db_->persist(member_apply_user);
            }

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
        std::vector<std::pair<std::string, std::string>> event_ids;
        auto transaction = MysqlClient::Transaction(db_);

        // 查询自己待处理的
        using query = ::odb::query<::linkchat::FriendApply>;
        auto results = db_->query<::linkchat::FriendApply>(query::respondent_id == user_id); 

        for(auto& result: results){
            // 将所有事件 id 都取出来，在服务层拼接上 sender info
            event_ids.push_back(std::make_pair(result.event_id(), result.user_id())); // 这里放的是申请人的userid
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
    std::vector<std::pair<std::string, std::string>> matches;
    // 昵称做子串模糊匹配，match 只能匹配分词后的完整词项，英文前缀无法命中，故用 wildcard
    std::vector<std::pair<std::string, std::string>> wildcards = {{"nickname", search_key}};
    auto value = es_->SelectTermAndMatch(term, matches, wildcards);

    std::vector<std::string> user_ids;
    for(int i = 0; i < count; i++){
        // 业务需要的是文档里的 user_id 字段，不是 ES 自动生成的文档 _id
        std::string user_id = es_->SourceField(value, i, "user_id");
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
            single_query::csm2::user_id != single_query::csm1::user_id); //当前用户，且非自己，视图会自动筛选掉 is_del_ == 1

        for(const auto& row : single_rows){
            ChatSessionInfo info;
            info.chat_session_id = row.chat_session_id;
            info.single_chat_friend_id = row.friend_id;
            // 单聊名称/头像在 service 层中通过 rpc 补全
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

        // 已经存在群名相同且成员完全相同的群聊时，不重复创建
        std::set<std::string> expected_members(member_id_list.begin(), member_id_list.end());
        expected_members.insert(user_id);

        // 先取出同名群聊的 id，避免在遍历结果时嵌套查询
        std::vector<std::string> same_name_session_ids;
        {
            using session_query = ::odb::query<::linkchat::ChatSession>;
            auto sessions = db_->query<::linkchat::ChatSession>(
                session_query::chat_session_type == ::linkchat::ChatSessionType::GROUP &&
                session_query::chat_session_name == chat_session_name);
            for(auto& session: sessions){
                same_name_session_ids.push_back(session.chat_session_id());
            }
        }

        using member_query = ::odb::query<::linkchat::ChatSessionMember>;
        for(auto& session_id: same_name_session_ids){
            std::set<std::string> members;
            {
                auto rows = db_->query<::linkchat::ChatSessionMember>(
                    member_query::chat_session_id == session_id);
                for(auto& row: rows){
                    members.insert(row.user_id());
                }
            }
            if(members == expected_members){
                return {false, "已存在同名且成员相同的群聊"};
            }
        }

        // 先创建群聊的元信息
        ::linkchat::ChatSession session;
        std::string session_id = service_prefix::chat_session_group_prefix + random_util::GetInstanceId();
        session.set_chat_session_id(session_id);
        session.set_chat_session_name(chat_session_name);
        session.set_chat_session_type(::linkchat::ChatSessionType::GROUP);
        db_->persist(session);

        // 加下来往群里加人
        auto creator = ::linkchat::ChatSessionMember(session_id, user_id);
        db_->persist(creator); // 添加创建人
        for(auto& mem_id: member_id_list){ // 添加其他成员
            if(mem_id == user_id) continue;
            ::linkchat::ChatSessionMember member(session_id, mem_id);
            db_->persist(member);
        }

        MysqlClient::Commit(transaction);
        return {true, "", session_id};
    }
    catch(const std::exception& e){
        return {false, e.what()};
    }
}

GetChatSessionMemberDTO FriendDao::GetChatSessionMember(const std::string& user_id, const std::string& chat_session_id){
    std::vector<std::string> users_id;
    try{
        auto transaction = MysqlClient::Transaction(db_);
        using query = ::odb::query<::linkchat::ChatSessionMember>;
        auto results = db_->query<::linkchat::ChatSessionMember>(
            query::chat_session_id == chat_session_id
        );

        // 这里获取成员，不包括自己
        for(auto& result: results){
            if(result.user_id() == user_id) continue;
            users_id.push_back(result.user_id());
        }

        MysqlClient::Commit(transaction);
        return {true, "", users_id};
    }
    catch(const std::exception& e){
        return {false, e.what()};
    }
}