#include <gflags/gflags.h>
#include <brpc/server.h>

#include "logging/logger.h"
#include "etcd/registry/service_registry.h"
#include "service_constants.h"
#include "controller/transmite_controller.h"

DEFINE_string(etcd_server_ip, "", "etcd server ip");
DEFINE_string(service_ip, "", "service ip");
DEFINE_int32(connection_timeout_seconds, 10,
             "time to wait when cannot connect to brpc server");
DEFINE_int32(idle_timeout_seconds, 5000,
             "connection without data transmition in these time closed");

DEFINE_int32(max_retry, 3, "max retry");
DEFINE_string(mysql_user, "root", "mysql user");
DEFINE_string(mysql_passwd, "123456", "mysql passwd");

DEFINE_string(amqp_broker, "", "amqp broker url");
DEFINE_string(amqp_exchange, "", "exchange name");
DEFINE_string(amqp_exchange_type, "", "exchange type");
DEFINE_string(amqp_queue, "", "queue name");
DEFINE_string(amqp_binding_key, "", "binding key");

DEFINE_int32(server_port, 9004, "brpc server port");

int main(int argc, char* argv[]){
    InitLogger();
    google::SetCommandLineOption("minloglevel",
                                 "2"); // 关闭bvar采集异常警告（不影响业务）
    logging::LoggingSettings logging_settings;
    logging_settings.logging_dest = logging::LOG_TO_NONE; // 不要输出日志
    google::SetCommandLineOption("flagfile", "conf/settings.conf");
    google::ParseCommandLineFlags(&argc, &argv, true);

    // 将服务注册到 etcd
    SvcRegistry registry(FLAGS_etcd_server_ip,
                         service_name::transmite_service,
                         FLAGS_service_ip);
    if (!registry.Registry()) {
        LOG_ERROR("Fail to register to Etcd");
        abort();
    }
    LOG_INFO("Successfully register to Etcd");

    //Rabbitmq 相关初始化
    MqClient::Ptr mq_client = std::make_shared<MqClient>(FLAGS_amqp_broker.c_str());
    DeclareSettings settings{FLAGS_amqp_exchange, FLAGS_amqp_exchange_type,
                             FLAGS_amqp_queue, FLAGS_amqp_binding_key};
    mq_client->Declare(settings);

    ::brpc::Server server;
    TransmiteDao::Ptr dao = std::make_shared<TransmiteDao>(
        FLAGS_mysql_user.c_str(), FLAGS_mysql_passwd.c_str()
    );
    TransmiteManager::Ptr transmite_manager = std::make_shared<TransmiteManager>(dao, FLAGS_amqp_exchange, FLAGS_amqp_binding_key, mq_client);
    TransmiteController transmite_server(transmite_manager);
    if(server.AddService(&transmite_server, brpc::SERVER_DOESNT_OWN_SERVICE) != 0){
        LOG_ERROR("Fail to add service to brpc server");
        abort();
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_seconds;
    if (server.Start(FLAGS_server_port, &options) != 0) {
        LOG_ERROR("{} brpc server start fail",
                  service_name::transmite_service);
        abort();
    }
    LOG_INFO("{} brpc server start successfully",
             service_name::transmite_service);
    server.RunUntilAskedToQuit();
    return 0;
}