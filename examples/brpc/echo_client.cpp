#include <thread>
#include <gflags/gflags.h>
#include <brpc/channel.h>

#include "echo.pb.h"

DEFINE_string(server_ip, "127.0.0.1:9000", "server address and port");
DEFINE_string(protocol, "baidu_std", "brpc protocol");
DEFINE_int32(port, 9000, "server port");
DEFINE_int32(connect_timeout_ms, 10000, "connect timeout");
DEFINE_int32(timeout_ms, 1000, "timeout in ms");
DEFINE_int32(max_retry, 10, "max retry count");

using namespace std::chrono_literals;

void AsyncCallback(::brpc::Controller* controller, ::echo::EchoResponse* response) {
    //在brpc异步调用中，需要手动释放连接，这里放到智能指针中
    std::unique_ptr<::brpc::Controller> controller_ptr(controller);
    std::unique_ptr<::echo::EchoResponse> response_ptr(new ::echo::EchoResponse());

    if (controller_ptr->Failed()) {
        std::cout << "Failed: " << controller_ptr->ErrorText() << std::endl;
    }
    else {
        std::cout << "Async Calling Successfully: " + response->resp_message() << std::endl;
    }
}

int main() {
    brpc::ChannelOptions channel_options;
    channel_options.connect_timeout_ms = FLAGS_connect_timeout_ms;
    channel_options.max_retry = FLAGS_max_retry;
    channel_options.timeout_ms = FLAGS_timeout_ms;
    channel_options.protocol = FLAGS_protocol;

    brpc::Channel channel;
    if (channel.Init(FLAGS_server_ip.c_str(), &channel_options) != 0) {
        std::cout << "Failed to init channel" << std::endl;
    }

    //如果这里不通过 stub 调用，需要自己处理网络连接等麻烦的前置步骤
    //stub 这里意思是占位的代码，在这里起到一个占位的作用。就是一个空壳子，实现“像调用本地方法一样调用远程方法”
    ::echo::EchoService_Stub stub(&channel);
    ::echo::EchoRequest request;
    for (int i = 0; i < 10; ++i) {
        request.set_req_message("this is a message from brpc client");
        auto* controller = new brpc::Controller();
        auto* response = new echo::EchoResponse();
        //stub.Echo(&controller, &request, &response, nullptr); //这里如果 done 设置为空，则为同步调用
        stub.Echo(controller, &request, response, ::brpc::NewCallback(AsyncCallback, controller, response)); //如果设置为异步调用，需要让 controller 和 response 放在堆上，并在回调中释放
    }

    std::this_thread::sleep_for(10s);
    return 0;
}
