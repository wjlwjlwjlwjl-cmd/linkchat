#include <gtest/gtest.h>

#include "../adapter/etcd/discover/service_discovery.hpp"

std::string etcd_server_ip = "http://etcd-service:2379";

auto online_cb = [](const std::string& svc_name, const std::string& svc_ip){
};
auto offline_cb = [](const std::string& svc_name, const std::string& svc_ip){
};

SvcDiscovery svc_discovery(etcd_server_ip, online_cb, offline_cb);

TEST(ServiceRegistryandDiscovery, ServiceRegistry){
    svc_discovery.Watch();
    std::cout << "Press Return to Quit" << std::endl;
    getchar();
}

int main(int argc, char* argv[]){
    init_logger();
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}