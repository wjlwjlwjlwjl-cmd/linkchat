#include "message_manager.h"

MessageManager::MessageManager(
    const std::string& etcd_server_ip, const std::string& protocol, int etcd_port, int connect_timeout_ms,
    int timeout_ms, int max_retry,
    const char* user, const char* passwd, const char* db_name, const char* host,
    unsigned int port, int connection_pool_size,
    const char* elastic_url, const char* index_name,
    const char* doc_type, const char* id){
    dao_ = std::make_shared<MessageDao>(user, passwd, db_name, host, port, connection_pool_size,
        elastic_url, index_name, doc_type, id);
}

bool MessageManager::PersistMessage(::linkchat::MessageInfo& info){
    // 取出消息的 metadata，并对所有消息的消息原信息进行存储
    std::string message_id = info.message_id();
    std::string chat_session_id = info.chat_session_id();
    std::string user_id = info.sender().user_id();
    ::linkchat::MessageType message_type = info.message().message_type();
    dao_->MsgMetadataStore(message_id, chat_session_id, user_id, message_type);

    if(message_type == ::linkchat::MessageType::STRING){
        if(!info.message().has_string_message()){
            LOG_WARN("string message {} has wrong content", info.message_id());
            return false;
        }
        std::string content = info.message().string_message().content();
        return dao_->MsgElasticStore(content, chat_session_id, message_id, user_id);
    }
    else{
        // 对于文件、图片类型的消息，上传阿里云对象存储
    }
    return true;
}
