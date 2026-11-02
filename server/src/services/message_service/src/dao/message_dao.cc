#include "message_dao.h"
#include "odb_mysql/mysql_client.h"
#include "logging/logger.h"
#include <stdexcept>

MessageDao::MessageDao(const char* user, const char* passwd, const char* db_name, const char* host,
    unsigned int port, int connection_pool_size, const char* elastic_url,
    const char* index_name, const char* doc_type, const char* id)
{
    db_ = MysqlClient::Create(user, passwd, db_name, host, port, connection_pool_size);

    es_ = std::make_shared<ElasticClient>(index_name, doc_type, id);
    ElasticClient::PropType props = {
        {"content", {{"type", "text"}, {"analyzer", "ikmax"}}},
        {"to_chat_session_id", {{"type", "keyword"}}},
        {"message_id", {{"type", "keyword"}}},
        {"from_user_id", {{"type", "keyword"}}}
    };
    if(!es_->CreateIndex(props)){
        abort();
    }
}

bool MessageDao::MsgMetadataStore(const std::string& message_id, const std::string& chat_session_id,
    const std::string& user_id, ::linkchat::MessageType& messageType, const std::string& content){
    MessageEntity msg;
    msg.set_message_id(message_id);
    msg.set_to_session_id(chat_session_id);
    msg.set_from_user_id(user_id);
    msg.set_message_type(static_cast<signed char>(messageType));
    msg.set_create_time(boost::posix_time::second_clock::local_time());
    msg.set_content(content); //对于文字类型，这里为文字内容；文件类型，这里为空（服务层设置为空）

    try{
        auto transaction = MysqlClient::Transaction(db_);
        db_->persist(msg);
        MysqlClient::Commit(transaction);
    }
    catch(std::exception& e){
        LOG_WARN("Failed to persist message {}: {}", message_id, e.what());
        return false;
    }
    return true;
}


bool MessageDao::MsgElasticStore(const std::string& content, const std::string& chat_session_id,
    const std::string& message_id, const std::string& from_user_id){
    ElasticClient::DataType data = {
        {"content", content},
        {"to_chat_session_id", chat_session_id},
        {"message_id", message_id},
        {"from_user_id", from_user_id}
    };
    return es_->AddData(data);
}

bool MessageDao::UpdateFileMetadata(const std::string& message_id, const std::string& file_name, 
    const std::string& file_id, int file_size){
    LOG_INFO("update file metadata of {}", message_id);
    try{
        auto transaction = MysqlClient::Transaction(db_);
        // 存在 sql 注入的问题，而且不优雅
        /*std::string sql = "UPDATE tb_msg set file_name=" + file_name
            + ", file_id=" + file_id
            + ", file_size=" + std::to_string(file_size)
            + " where message_id=" + message_id + ";";*/

        using query = odb::query<MessageEntity>;
        auto entity = db_->query_one<MessageEntity>(query::message_id == message_id);
        if(!entity){
            LOG_WARN("Message id: {}, cannot be found", message_id);
            return false;
        }
        entity->set_file_id(file_id);
        entity->set_file_name(file_name);
        entity->set_file_size(file_size);

        auto& section = entity->file_metadata_section();
        section.change();
        db_->update(*entity);

        MysqlClient::Commit(transaction);
    }
    catch(const std::exception& e){
        LOG_WARN("failed to update file msg {}: {}", file_id, e.what());
        return false;
    }
    return true;
}

// 获取某个会话 chat_session_id 中，一个用户 user_id，在 cur_time 前的 msg_count 条消息
std::vector<::linkchat::MessageInfo> MessageDao::GetRecentMsg(const std::string& chat_session_id, int msg_count, 
    int64_t cur_time, const std::string& user_id){
    std::vector<::linkchat::MessageInfo> msgs;
    try{
        auto transaction = MysqlClient::Transaction(db_);

        boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
        boost::posix_time::ptime query_time = epoch + boost::posix_time::minutes(cur_time);

        if(msg_count < 0) msg_count = 10;
        else if(msg_count > 100) msg_count = 100;

        using query = odb::query<MessageEntity>;
        using result = odb::result<MessageEntity>;

        result r = db_->query<MessageEntity>((query::to_session_id == chat_session_id
            && query::create_time <= query_time
            && query::from_user_id == user_id)
            + " ORDER BY " + query::create_time + " DESC LIMIT " + std::to_string(msg_count));

        constexpr int kMsgTypeString = 0;
        constexpr int kMsgTypeFile   = 1;

        for(auto& msg: r){
            ::linkchat::MessageInfo msg_info;
            msg_info.set_message_id(msg.message_id());
            msg_info.set_chat_session_id(msg.to_session_id());
            msg_info.mutable_sender()->set_user_id(msg.from_user_id());

            if(msg.message_type() == kMsgTypeString){
                msg_info.mutable_message()->set_message_type(::linkchat::MessageType::STRING);
                if(msg.has_content())
                    msg_info.mutable_message()->mutable_string_message()->set_content(msg.content().get());
            }
            else if(msg.message_type() == kMsgTypeFile){
                msg_info.mutable_message()->set_message_type(::linkchat::MessageType::FILE);
                if(msg.has_file_id())
                    msg_info.mutable_message()->mutable_file_message()->set_file_id(msg.file_id().get());
                if(msg.has_file_name())
                    msg_info.mutable_message()->mutable_file_message()->set_file_name(msg.file_name().get());
                if(msg.has_file_size())
                    msg_info.mutable_message()->mutable_file_message()->set_file_size(msg.file_size().get());
            }
            else{
                LOG_WARN("Unknown message type {}: {}", msg.message_id(), msg.message_type());
                continue;
            }
            
            msgs.emplace_back(std::move(msg_info));
        }

        MysqlClient::Commit(transaction);
    }
    catch(const std::exception& e){
        LOG_WARN("failed to get recent msg {}, {}: {}", chat_session_id, user_id, e.what());
    }
    return msgs;
}

// 获取会话 chat_session_id，从 start_time 开始，到 end_time 结束的 user_id 的消息
std::vector<::linkchat::MessageInfo> MessageDao::GetHistroyMsg(const std::string& chat_session_id, int64_t start_time, 
    int64_t end_time, const std::string& user_id) {
    std::vector<::linkchat::MessageInfo> msgs;
    try {
        auto transaction = MysqlClient::Transaction(db_);

        boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
        boost::posix_time::ptime query_start_time = epoch + boost::posix_time::minutes(start_time);
        boost::posix_time::ptime query_end_time   = epoch + boost::posix_time::minutes(end_time);

        using query = odb::query<MessageEntity>;
        using result = odb::result<MessageEntity>;

        // 查询区间：[start_time, end_time]，并按时间正序（历史消息一般从早到晚展示）
        auto q = query::to_session_id == chat_session_id
               && query::create_time >= query_start_time
               && query::create_time <= query_end_time
               && query::from_user_id == user_id;

        result r = db_->query<MessageEntity>(
            q + " ORDER BY " + query::create_time + " ASC");

        constexpr int kMsgTypeString = 0;
        constexpr int kMsgTypeFile   = 1;

        for (auto& msg : r) {
            ::linkchat::MessageInfo msg_info;
            msg_info.set_message_id(msg.message_id());
            msg_info.set_chat_session_id(msg.to_session_id());
            msg_info.mutable_sender()->set_user_id(msg.from_user_id());

            if (msg.message_type() == kMsgTypeString) {
                msg_info.mutable_message()->set_message_type(::linkchat::MessageType::STRING);
                if (msg.has_content())
                    msg_info.mutable_message()->mutable_string_message()->set_content(msg.content().get());
            } else if (msg.message_type() == kMsgTypeFile) {
                msg_info.mutable_message()->set_message_type(::linkchat::MessageType::FILE);
                if (msg.has_file_id())
                    msg_info.mutable_message()->mutable_file_message()->set_file_id(msg.file_id().get());
                if (msg.has_file_name())
                    msg_info.mutable_message()->mutable_file_message()->set_file_name(msg.file_name().get());
                if (msg.has_file_size())
                    msg_info.mutable_message()->mutable_file_message()->set_file_size(msg.file_size().get());
            } else {
                LOG_WARN("Unknown message type {}: {}", msg.message_id(), msg.message_type());
                continue;
            }

            msgs.push_back(std::move(msg_info));
        }

        MysqlClient::Commit(transaction);
    } catch (const std::exception& e) {
        LOG_WARN("failed to get history msg {}, {}, [{} - {}]: {}", chat_session_id, user_id, start_time, end_time, e.what());
    }
    return msgs;
}

std::vector<::linkchat::MessageInfo> MessageDao::MsgSearch(const std::string& chat_session_id, const std::string& user_id,
    const std::string& search_key){
    std::vector<::linkchat::MessageInfo> msgs;
    // 必传参数为空时直接返回，避免 ES 侧跳过过滤条件导致跨会话/全量检索
    if(chat_session_id.empty() || search_key.empty()){
        return msgs;
    }
    /*
        "properties" : {
            "content" : {
                "analyzer" : "ikmax",
                "type" : "text"
            },
            "from_user_id" : {
                "type" : "keyword"
            },
            "message_id" : {
                "type" : "keyword"
            },
            "to_chat_session_id" : {
                "type" : "keyword"
            }
        }
    */
    
    std::vector<std::pair<std::string, std::string>> terms = {
        {"from_user_id", user_id},
        {"to_chat_session_id", chat_session_id}
    };
    std::vector<std::pair<std::string, std::string>> matches = {
        {"content", search_key}
    };
    auto resp_json = es_->SelectTermAndMatch(terms, matches);

    if (resp_json.isNull() || !resp_json.isObject()) {
        LOG_ERROR("es response is null or not object");
        return msgs; // 或者返回空集合
    }

    if (!resp_json.isMember("hits") || !resp_json["hits"].isObject()) {
        LOG_WARN("es response has no hits");
        return msgs;
    }

    const Json::Value& hits_obj = resp_json["hits"];

    if (!hits_obj.isMember("hits") || !hits_obj["hits"].isArray()) {
        LOG_WARN("es response hits.hits is not an array");
        return msgs;
    }

    const Json::Value& hits_array = hits_obj["hits"];

    std::vector<std::string> message_ids;
    for (const auto& hit : hits_array) {
        if (!hit.isMember("_source") || !hit["_source"].isObject()) {
            continue;
        }
        const Json::Value& source = hit["_source"];

        if (source.isMember("message_id") && source["message_id"].isString()) {
            std::string msg_id = source["message_id"].asString();
            message_ids.push_back(msg_id);
        }
    }

    try{
        auto transaction = MysqlClient::Transaction(db_);

        for(auto& msg_id: message_ids){
            ::linkchat::MessageInfo msg_info;

            auto msg = db_->query_one<MessageEntity>(odb::query<MessageEntity>::message_id == msg_id);
            if(msg == nullptr){
                continue;
            }

            msg_info.set_message_id(msg->message_id());
            msg_info.set_chat_session_id(msg->to_session_id());
            msg_info.mutable_sender()->set_user_id(msg->from_user_id());

            msg_info.mutable_message()->set_message_type(::linkchat::MessageType::STRING);
            if (msg->has_content()){
                msg_info.mutable_message()->mutable_string_message()->set_content(msg->content().get());
            }
            msgs.push_back(std::move(msg_info));
        }

        MysqlClient::Commit(transaction);
    }
    catch(const std::exception& e){
        LOG_WARN("failed to get msg by es result: {}", e.what());
    }

    return msgs;
}