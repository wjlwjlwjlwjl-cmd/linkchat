#include <gtest/gtest.h>

#include "../adapter/etcd/discover/service_discovery.h"

std::string g_etcd_server_ip = "http://etcd-service:2379";

auto g_on_online = [](const std::string& svc_name, const std::string& svc_ip) {
};
auto g_on_offline = [](const std::string& svc_name, const std::string& svc_ip) {
};

SvcDiscovery g_svc_discovery(g_etcd_server_ip, g_on_online, g_on_offline);

TEST(ServiceRegistryAndDiscovery, ServiceDiscovery) {
    g_svc_discovery.Watch();
    std::cout << "Press Return to Quit" << std::endl;
    getchar();
}

int main(int argc, char* argv[]) {
    InitLogger();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
