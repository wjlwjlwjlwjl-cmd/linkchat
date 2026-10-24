#include <brpc/server.h>
#include <gflags/gflags.h>
#include <butil/logging.h>

#include "echo.pb.h"

DEFINE_int32(idle_timeout, 100, "Idle timeout in seconds");
DEFINE_int32(server_port, 9000, "Port of server");

class EchoService : public echo::EchoService {
public:
    void Echo(google::protobuf::RpcController* controller,
                         const ::echo::EchoRequest* request,
                         ::echo::EchoResponse* response,
                         ::google::protobuf::Closure* done) override {
        brpc::ClosureGuard closure(done); // 需要手动调用 done->Run() 完成后续的序列化和推送，使用 CloseureGuard 通过 RAII 的方式保证完成调用
        std::cout << "echo::EchoRequest::Echo() ==>" << request->req_message() << std::endl;
        response->set_resp_message("this is a message returned by brpc server");
    }
};

int main(int argc, char* argv[]) {
    google::SetCommandLineOption("minloglevel", "2"); //关闭bvar采集异常警告（不影响业务）
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_NONE; //不要输出日志
    google::ParseCommandLineFlags(&argc, &argv, true);

    brpc::Server server;
    EchoService service;

    if (server.AddService(&service, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
        std::cout << "Failed to add service" << std::endl;
    }

    brpc::ServerOptions options;
    options.idle_timeout_sec = FLAGS_idle_timeout;

    if (server.Start(FLAGS_server_port, &options) != 0) {
        std::cout << "Failed to start server" << std::endl;
    }

    server.RunUntilAskedToQuit(); //阻塞运行brpc服务器

    return 0;
}