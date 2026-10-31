#include <odb/core.hxx>
#include <odb/database.hxx>
#include <elasticlient/client.h>
#include <cpr/response.h>

#include "message.pb.h"
#include "elastic/elastic_client.h"
#include "message_entity-odb.hxx"
#include "message_entity.h"

class MessageDao{
public:
    using Ptr = std::shared_ptr<MessageDao>;
    MessageDao(const char* user, const char* passwd, const char* db_name = "msg_db", const char* host = "mysql-service", 
        unsigned int port = 3306, int connection_pool_size = 5, 
        const char* elastic_url = "http://elastic:123456@elasticsearch-service:9200/", 
        const char* index_name = "message", const char* doc_type = "_doc", const char* id = "");

    bool MsgMetadataStore(const std::string& message_id, const std::string& chat_session_id, 
        const std::string user_id, ::linkchat::MessageType& messageType);

    // content 进行检索，并且通过 chat_session_id 进行对话过滤，因此需要插入 content（text），chat_session_id（keyword）
    bool MsgElasticStore(const std::string& content, const std::string& chat_session_id,
        const std::string& message_id, const std::string& from_user_id);
private:
    std::shared_ptr<odb::database> db_;

    std::shared_ptr<ElasticClient> es_;
};