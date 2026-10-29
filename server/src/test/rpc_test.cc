// 这里启动测试的 EchoServer，作为服务器

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "../adapter/rpc/callback_factory.h"
#include "../adapter/rpc/svc_channels.h"
#include "../adapter/logging/logger.h"
#include "echo.pb.h"

using namespace std::chrono_literals;

CallbackFactory g_callback_factory;
SvcChannels g_svc_channels;

TEST(RpcWrapper, AddNodeAndGetChannel) {
    g_svc_channels.AddWatch("EchoService");
    g_svc_channels.AddNode("EchoService", "127.0.0.1:9000");

    auto channel = g_svc_channels.GetChannel("EchoService");

    auto request = std::make_shared<::echo::EchoRequest>();
    auto response = std::make_shared<::echo::EchoResponse>();
    auto controller = std::make_shared<::brpc::Controller>();

    auto closure = g_callback_factory.CreateCallback([&]() {
        if (controller->Failed()) {
            LOG_ERROR("rpc fail: {}", controller->ErrorText());
        }
        else {
            LOG_INFO("rpc success: {}", response->resp_message());
        }
    });

    ::echo::EchoService::Stub echo_service_stub(channel.get());
    echo_service_stub.Echo(controller.get(), request.get(), response.get(), closure);

    auto opt_ip = g_svc_channels.GetAddr("EchoService");
    LOG_INFO("opt_ip: {}", opt_ip.has_value() ? opt_ip.value() : "none");
    std::this_thread::sleep_for(5s);
}

TEST(RpcWrapper, RemoveNode) {
    g_svc_channels.DelNode("EchoService", "127.0.0.1:9000");
    auto channel = g_svc_channels.GetChannel("EchoService");
    ASSERT_EQ(channel, nullptr);
}

int main(int argc, char** argv) {
    InitLogger();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
