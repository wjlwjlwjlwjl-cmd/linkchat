#include <functional>
#include <gflags/gflags.h>
#include <brpc/server.h>
#include <thread>

#include "etcd/registry/service_registry.h"
#include "logging/logger.h"
#include "service_name.h"
#include "amqp/mq_client.h"
#include "amqp/consumer.h"
#include "server/message_server.h"

DEFINE_string(etcd_server_ip, "", "etcd server ip");
DEFINE_string(service_ip, "", "service ip");
DEFINE_int32(connection_timeout_seconds, 10, "time to wait when cannot connect to brpc server");
DEFINE_int32(idle_timeout_seconds, 5000, "connection without data transmition in these time closed");
DEFINE_int32(max_retry, 3, "max retry");
DEFINE_string(mysql_user, "root", "mysql user");
DEFINE_string(mysql_passwd, "123456", "mysql passwd");
DEFINE_string(amqp_broker, "", "amqp broker url");
DEFINE_string(amqp_exchange, "", "exchange name");
DEFINE_string(amqp_exchange_type, "", "exchange type");
DEFINE_string(amqp_queue, "", "queue name");
DEFINE_string(amqp_binding_key, "", "binding key");
DEFINE_int32(server_port, 9000, "brpc server port");

int main(int argc, char* argv[]){
    InitLogger();
    google::SetCommandLineOption("minloglevel", "2"); //关闭bvar采集异常警告（不影响业务）
    logging::LoggingSettings logging_settings;
    logging_settings.logging_dest = logging::LOG_TO_NONE; //不要输出日志
    google::SetCommandLineOption("flagfile", "conf/settings.conf");
    google::ParseCommandLineFlags(&argc, &argv, true);

    // 将服务注册到 etcd
    SvcRegistry registry(FLAGS_etcd_server_ip, service_name::message_storage_service, FLAGS_service_ip);
    if(!registry.Registry()){
        LOG_ERROR("Fail to register to Etcd");
        abort();
    }
    LOG_INFO("Successfully register to Etcd");

    //构建 brpc server
    brpc::Server server;
    MessageServer message_server(FLAGS_etcd_server_ip.c_str(), "baidu_std", FLAGS_connection_timeout_seconds,
        FLAGS_idle_timeout_seconds, FLAGS_max_retry, FLAGS_mysql_user.c_str(), FLAGS_mysql_passwd.c_str());

    //去监听 amqp
    MqClient mq_client(FLAGS_amqp_broker);
    DeclareSettings settings{FLAGS_amqp_exchange, FLAGS_amqp_exchange_type, FLAGS_amqp_queue, FLAGS_amqp_binding_key};
    mq_client.Declare(settings);
    std::function<bool(const char*, int)> cb = std::bind(&MessageServer::OnMessage, &message_server, std::placeholders::_1, std::placeholders::_2);
    mq_client.Consume(FLAGS_amqp_queue, cb);
    LOG_INFO("Listening rabbit queue: {}", FLAGS_amqp_queue);

    if(server.AddService(&message_server, brpc::SERVER_DOESNT_OWN_SERVICE) != 0){
        LOG_ERROR("Fail to add service to brpc server");
        abort();
    }

    //启动 brpc server
    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_seconds;
    if(server.Start(FLAGS_server_port, &options) != 0){
        LOG_ERROR("{} brpc server start fail", service_name::message_storage_service);
    }
    LOG_INFO("{} brpc server start successfully", service_name::message_storage_service);
    server.RunUntilAskedToQuit();

    return 0;
}

/*
测试文字类型
{
  "message_id": "msg_10001",
  "chat_session_id": "session_888",
  "sender": {
    "user_id": "user_123",
  },
  "message": {
    "message_type": "STRING",
    "string_message": {
      "content": "你好，这是一条测试文字消息！"
    }
  }
}

测试文件类型
{
  "message_id": "msg_test_file_001",
  "chat_session_id": "session_test_888",
  "sender": {
    "user_id": "user_123"
  },
  "message": {
    "message_type": "FILE",
    "file_message": {
      "file_id": "file_id_test_999",
      "file_name": "hello.txt",
      "file_size": "15",
      "file_content": "SGVsbG8gUmFiYml0TVEh"
    }
  }
}


*/