#pragma once
#include <odb/core.hxx>
#include <odb/database.hxx>
#include <elasticlient/client.h>
#include <cpr/response.h>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <string>
#include <vector>

#include "message_info.h"
#include "elastic/elastic_client.h"
#include "message_entity-odb.hxx"
#include "message_entity.h"

class MessageDao{
public:
    using Ptr = std::shared_ptr<MessageDao>;
    MessageDao(const char* user, const char* passwd, const char* db_name = "linkchat", const char* host = "mysql-service", 
        unsigned int port = 3306, int connection_pool_size = 5, 
        const char* elastic_url = "http://elastic:123456@elasticsearch-service:9200/", 
        const char* index_name = "message", const char* doc_type = "_doc");

    bool MsgMetadataStore(const std::string& message_id, const std::string& chat_session_id, 
        const std::string& from_user_id, MessageType message_type, const std::string& content);

    // content 进行检索，并且通过 chat_session_id 进行对话过滤，因此需要插入 content（text），chat_session_id（keyword）
    bool MsgElasticStore(const std::string& content, const std::string& chat_session_id,
        const std::string& message_id, const std::string& from_user_id);
    
    //在调用完文件存储后，更新file_name、file_id、file_size
    bool UpdateFileMetadata(const std::string& message_id, const std::string& file_name, 
        const std::string& file_id, int file_size);

    /// @brief 获取最近 N 条消息
    /// @param chat_session_id  聊天会话 id
    /// @param msg_count  期望获取的条数
    /// @param cur_time  当前时间（64位时间戳）
    /// @param user_id  用户id
    /// @return  获取到的消息列表（类型 MessageInfo）
    std::vector<MessageInfo> GetRecentMsg(const std::string& chat_session_id, int msg_count, 
        int64_t cur_time, const std::string& user_id);

    std::vector<MessageInfo> GetHistroyMsg(const std::string& chat_session_id, int64_t start_time, 
        int64_t end_time, const std::string& user_id);

    std::vector<MessageInfo> MsgSearch(const std::string& chat_session_id, const std::string& user_id,
        const std::string& search_key);
private:
    std::shared_ptr<odb::database> db_;

    std::shared_ptr<ElasticClient> es_;
};