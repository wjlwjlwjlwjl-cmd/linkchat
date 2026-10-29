#include <gtest/gtest.h>

#include "../adapter/etcd/registry/service_registry.h"

std::string g_etcd_server_ip = "http://etcd-service:2379";
std::string g_svc_name = "EchoService";
std::string g_svc_ip = "127.0.0.1:9000";

SvcRegistry g_svc_registry(g_etcd_server_ip, g_svc_name, g_svc_ip);

TEST(ServiceRegistryAndDiscovery, ServiceRegistry) {
    g_svc_registry.Registry();
    std::cout << "Press Return to Quit" << std::endl;
    getchar();
}

int main(int argc, char* argv[]) {
    InitLogger();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
