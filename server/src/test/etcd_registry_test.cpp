#include <gtest/gtest.h>

#include "../adapter/etcd/registry/service_registry.hpp"

std::string etcd_server_ip = "http://etcd-service:2379";
std::string svc_name = "EchoService";
std::string svc_ip = "127.0.0.1:9000";

SvcRegistry svc_registry(etcd_server_ip, svc_name, svc_ip);
//SvcDiscovery svc_discovery;

TEST(ServiceRegistryandDiscovery, ServiceRegistry){
    svc_registry.Registry();
    std::cout << "Press Return to Quit" << std::endl;
    getchar();
}

int main(int argc, char* argv[]){
    init_logger();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}