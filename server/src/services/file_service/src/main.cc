#include <iostream>
#include <gflags/gflags.h>
#include <brpc/server.h>

#include "etcd/registry/service_registry.h"
#include "logging/logger.h"
#include "server/file_server.h"
#include "service/file_manager.h"
#include "dao/OssDao.h"

DEFINE_string(aliyun_access_id, "", "aliyun access id");
DEFINE_string(aliyun_access_key, "", "aliyun access key");
DEFINE_string(aliyun_endpoint, "", "Endpoint");
DEFINE_string(aliyun_region, "", "Region");
DEFINE_string(aliyun_bucket_name, "", "Bucket Name");
DEFINE_string(etcd_server_ip, "", "etcd server ip");
DEFINE_string(service_name, "", "service name");
DEFINE_string(service_ip, "", "service ip");
DEFINE_int32(idle_timeout_seconds, 100, "connection without data transmition in these time closed");
DEFINE_int32(server_port, 9000, "brpc server port");

int main(int argc, char* argv[]){
    InitLogger();
    google::SetCommandLineOption("minloglevel", "2"); //关闭bvar采集异常警告（不影响业务）
    logging::LoggingSettings logging_settings;
    logging_settings.logging_dest = logging::LOG_TO_NONE; //不要输出日志

    google::SetCommandLineOption("flagfile", "conf/settings.conf");

    google::ParseCommandLineFlags(&argc, &argv, true);

    auto dao = std::make_shared<OssDao>(FLAGS_aliyun_endpoint, FLAGS_aliyun_region, FLAGS_aliyun_bucket_name, FLAGS_aliyun_access_id, 
        FLAGS_aliyun_access_key);
    auto service = std::make_shared<FileManager>(dao);
    FileServer file_server(service);

    SvcRegistry registry(FLAGS_etcd_server_ip, FLAGS_service_name, FLAGS_service_ip);
    if(!registry.Registry()){
        LOG_ERROR("Fail to register to Etcd");
        abort();
    }
    LOG_INFO("Successfully register to Etcd");

    brpc::Server server;

    if(server.AddService(&file_server, brpc::SERVER_DOESNT_OWN_SERVICE) != 0){
        LOG_ERROR("Fail to add service to brpc server");
        abort();
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout_seconds;
    if(server.Start(FLAGS_server_port, &options) != 0){
        LOG_ERROR("{} brpc server start fail", FLAGS_service_name);
    }
    LOG_INFO("{} brpc server start successfully", FLAGS_service_name);
    server.RunUntilAskedToQuit();

    return 0;
}