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
    const std::string user_id, ::linkchat::MessageType& messageType){
    MessageEntity msg;
    msg.set_message_id(message_id);
    msg.set_to_session_id(chat_session_id);
    msg.set_from_user_id(user_id);
    msg.set_message_type(static_cast<signed char>(messageType));
    msg.set_create_time(boost::posix_time::second_clock::local_time());

    try{
        auto transaction = MysqlClient::Transaction(db_);
        db_->persist(msg);
        MysqlClient::Commit(transaction);
    }
    catch(std::exception& e){
        LOG_WARN("Failed to persist message {}", message_id);
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
        LOG_WARN("{} failed to update file msg", e.what());
        return false;
    }
    return true;
}
