// 需要提供的数据服务：
// 1. 消息元数据存入数据库
// 2. 文件、图片存入oss
// 3. 文本信息直接存入 ES
// 4. 提供 ES、数据库 检索接口
//
// !!!! 需构建项目，在build相应目录生成 .sql 建表文件，需要建库建表后运行


#include "dao/message_dao.h"
#include "message.pb.h"
#include "file.pb.h"
#include "etcd/discover/service_discovery.h"

class MessageManager{
public:
    MessageManager(
        const std::string& etcd_server_ip, const std::string& protocol, int etcd_port, int connect_timeout_ms,
        int timeout_ms, int max_retry,
        const char* user, const char* passwd, const char* db_name = "msg_db", const char* host = "mysql-service",
        unsigned int port = 3306, int connection_pool_size = 5,
        const char* elastic_url = "http://elastic:123456@elasticsearch-service:9200/",
        const char* index_name = "message", const char* doc_type = "_doc", const char* id = "");

    //消息持久化
    bool PersistMessage(::linkchat::MessageInfo& info);
private:
    std::shared_ptr<MessageDao> dao_;
};
