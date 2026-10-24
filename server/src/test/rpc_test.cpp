// 这里启动测试的 EchoServer，作为服务器

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "../adapter/rpc/callback_factory.hpp"
#include "../adapter/rpc/svc_channels.hpp"
#include "../adapter/logging/logger.hpp"
#include "echo.pb.h"

using namespace std::chrono_literals;

CallbackFactory callback_factory;
SvcChannels svc_channels;

TEST(RPC封装测试, 添加节点并调用) {
    svc_channels.AddWatch("EchoService");
    svc_channels.AddNode("EchoService", "127.0.0.1:9000");

    auto channel = svc_channels.GetChannel("EchoService");

    auto request = std::make_shared<::echo::request>();
    auto response = std::make_shared<::echo::response>();
    auto controller = std::make_shared<::brpc::Controller>();

    auto closure = callback_factory.CreateCallback([&]() {
        if (controller->Failed()) {
            LOGGER_ERROR("rpc fail: {}", controller->ErrorText());
        }
        else {
            LOGGER_INFO("rpc success: {}", response->resp_message());
        }
    });

    ::echo::EchoService::Stub echo_service_stub(channel.get());
    echo_service_stub.Echo(controller.get(), request.get(), response.get(), closure);

    auto opt_ip = svc_channels.GetAddr("EchoService");
    LOGGER_INFO("opt_ip: {}", opt_ip.has_value() ? opt_ip.value() : "none");
    std::this_thread::sleep_for(5s);
}

TEST(RPC封装测试, 删除节点) {
    svc_channels.DelNode("EchoService", "127.0.0.1:9000");
    auto channel = svc_channels.GetChannel("EchoService");
    ASSERT_EQ(channel, nullptr);
}

int main(int argc, char** argv) {
    init_logger();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
