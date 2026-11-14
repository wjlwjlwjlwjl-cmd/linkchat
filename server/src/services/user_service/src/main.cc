#include <gflags/gflags.h>
#include <brpc/server.h>

#include "logging/logger.h"
#include "etcd/registry/service_registry.h"
#include "service_constants.h"
#include "dao/user_dao.h"
#include "service/user_manager.h"
#include "controller/user_controller.h"

DEFINE_string(etcd_server_ip, "", "etcd server ip");
DEFINE_string(service_ip, "", "service ip");
DEFINE_string(protocol, "", "rpc protocol");
DEFINE_int32(connection_timeout_seconds, 10, "time to wait when cannot connect to brpc server");
DEFINE_int32(idle_timeout_seconds, 5000, "connection without data transmition in these time closed");
DEFINE_int32(max_retry, 3, "max retry");

DEFINE_string(mysql_user, "root", "mysql user");
DEFINE_string(mysql_passwd, "123456", "mysql passwd");
DEFINE_string(mysql_db, "", "mysql db");

DEFINE_int32(server_port, 9002, "brpc server port");

DEFINE_string(smtp_host, "", "smtp host");
DEFINE_string(smtp_password, "", "smtp password");
DEFINE_string(smtp_user, "", "smtp user");

int main(int argc, char* argv[]){
    InitLogger();
    google::SetCommandLineOption("minloglevel", "2"); //关闭bvar采集异常警告（不影响业务）
    logging::LoggingSettings logging_settings;
    logging_settings.logging_dest = logging::LOG_TO_NONE; //不要输出日志
    ::google::SetCommandLineOption("flagfile", "conf/settings.conf"); //设置日志目录
    ::google::ParseCommandLineFlags(&argc, &argv, true);

    SvcRegistry registry(FLAGS_etcd_server_ip, service_name::user_service, FLAGS_service_ip);
    if(!registry.Registry()){
        LOG_ERROR("Fail to Register to Etcd");
        abort();
    }
    LOG_INFO("Successfully register to Etcd");

    UserDao::Ptr dao = std::make_shared<UserDao>(
        FLAGS_mysql_user.c_str(), FLAGS_mysql_passwd.c_str(), FLAGS_mysql_db.c_str());
    redis_factory::RedisConnectionInfo redis_info;
    UserManager::Ptr manager = std::make_shared<UserManager>(
        dao, redis_info, FLAGS_smtp_host, FLAGS_smtp_user, FLAGS_smtp_password,
        FLAGS_etcd_server_ip, FLAGS_protocol, FLAGS_connection_timeout_seconds * 1000,
        FLAGS_idle_timeout_seconds * 1000, FLAGS_max_retry
    );
    UserController user_server(manager);

    brpc::Server server;
    if(server.AddService(&user_server, ::brpc::SERVER_DOESNT_OWN_SERVICE) != 0){
        LOG_ERROR("Fail to add service to brpc server");
        abort();
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_seconds;
    if(server.Start(FLAGS_server_port, &options) != 0){
        LOG_ERROR("Fail to start brpc server {} on port {}", service_name::user_service, FLAGS_server_port);
        abort();
    }
    LOG_INFO("{} brpc server start successfully", service_name::user_service);
    server.RunUntilAskedToQuit();

    return 0;
}