// 网关目前只提供一个实例，负载均衡放在 etcd 实例获取中

#include <brpc/channel.h>
#include <gflags/gflags.h>
#include <logging/logger.h>
#include "gateway.h"
#include "etcd/discover/service_discovery.h"

DEFINE_string(gateway_listen_ip, "0.0.0.0", "Gateway listen IP address");
DEFINE_int32(gateway_port, 9000, "Gateway listen port");

DEFINE_string(etcd_server_ip, "", "etcd server ip");
DEFINE_int32(connection_timeout_seconds, 10, "time to wait when cannot connect to brpc server");
DEFINE_int32(timeout_seconds, 5000, "connection without data transmition in these time closed");
DEFINE_string(protocol, "baidu_std", "brpc protocol");
DEFINE_int32(max_retry, 3, "max retry");

int main(int argc, char* argv[]){
    InitLogger();
    google::SetCommandLineOption("minloglevel", "2"); //关闭bvar采集异常警告（不影响业务）
    ::logging::LoggingSettings logging_settings;
    logging_settings.logging_dest = logging::LOG_TO_NONE; //不要输出日志
    ::google::SetCommandLineOption("flagfile", "conf/settings.conf");
    ::google::ParseCommandLineFlags(&argc, &argv, true);

    auto options = std::make_shared<brpc::ChannelOptions>();
    options->connect_timeout_ms = FLAGS_connection_timeout_seconds * 1000;
    options->timeout_ms = FLAGS_timeout_seconds * 1000;
    options->max_retry = FLAGS_max_retry;
    options->protocol = FLAGS_protocol;

    SvcDiscovery::ModCallback svc_online = 
        [](const std::string& svc_name, const std::string& svc_ip){};
    SvcDiscovery::ModCallback svc_offline = 
        [](const std::string& svc_name, const std::string& svc_ip){};
    SvcDiscovery::Ptr svc_discovery = 
        std::make_shared<SvcDiscovery>(FLAGS_etcd_server_ip, std::move(svc_online), std::move(svc_offline));

    Gateway gateway(options, svc_discovery);
    gateway.run(FLAGS_gateway_listen_ip.c_str(), FLAGS_gateway_port);
    return 0;
}