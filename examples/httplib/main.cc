#include <httplib.h>

void helloWorld(const httplib::Request& request, httplib::Response& response) {
    std::cout << request.path << std::endl;
    std::cout << request.method << std::endl;
    std::cout << request.body << std::endl;
    for (auto it: request.headers) {
        std::cout << it.first << "-" << it.second << std::endl;
    }
    for (auto it: request.params) {
        std::cout << it.first << "-" << it.second << std::endl;
    }
    response.set_content("this is a response returned from httplib server", "text/plain");
    response.status = 200;
}

int main() {
    httplib::Server server;
    server.Get("/hello_world", helloWorld);
    server.Get(R"(/test_path_variable/(\d+))", [&](const httplib::Request& request, httplib::Response& response) {
        std::cout << request.path << std::endl;
        std::cout << request.method << std::endl;
        std::cout << request.body << std::endl;
        for (auto it: request.matches) {
            std::cout << it << std::endl;
        }
    });
    server.set_mount_point("/", "../webroot");
    server.listen("0.0.0.0", 8080);
    return 0;
}