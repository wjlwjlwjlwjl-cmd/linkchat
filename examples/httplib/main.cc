#include <httplib.h>

void HelloWorld(const httplib::Request& request, httplib::Response& response) {
    std::cout << request.path << std::endl;
    std::cout << request.method << std::endl;
    std::cout << request.body << std::endl;
    for (auto header : request.headers) {
        std::cout << header.first << "-" << header.second << std::endl;
    }
    for (auto param : request.params) {
        std::cout << param.first << "-" << param.second << std::endl;
    }
    response.set_content("this is a response returned from httplib server", "text/plain");
    response.status = 200;
}

int main() {
    httplib::Server server;
    server.Get("/hello_world", HelloWorld);
    server.Get(R"(/test_path_variable/(\d+))", [&](const httplib::Request& request, httplib::Response& response) {
        std::cout << request.path << std::endl;
        std::cout << request.method << std::endl;
        std::cout << request.body << std::endl;
        for (auto match : request.matches) {
            std::cout << match << std::endl;
        }
    });
    server.set_mount_point("/", "../webroot");
    server.listen("0.0.0.0", 9000);
    return 0;
}
