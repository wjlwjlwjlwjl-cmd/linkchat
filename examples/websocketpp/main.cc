#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> WsServer;
typedef WsServer::message_ptr MessagePtr;

void OnOpen(WsServer* server, websocketpp::connection_hdl handle) {
    std::cout << "ws_server open" << std::endl;
}

void OnClose(WsServer* server, websocketpp::connection_hdl handle) {
    std::cout << "ws_server close" << std::endl;
}

void OnFail(WsServer* server, websocketpp::connection_hdl handle) {
    std::cout << "ws_server fail" << std::endl;
}

void OnMessage(WsServer* server, websocketpp::connection_hdl handle, MessagePtr msg) {
    std::cout << "ws_server get message" << msg->get_payload() << std::endl;
    server->send(handle, "this is a message returned from ws_server", websocketpp::frame::opcode::text); //通过四比特位的标签，来指示数据用途
}

//这里是设置了 http 方法，但是对于不同客户端处理方式，可能会出现 ws_server fail，但是回调正常的，消息传递也正常（点名 ApiFox）
void OnHttp(WsServer* server, websocketpp::connection_hdl handle) {
    std::cout << "ws_server get http" << std::endl;
    std::stringstream ss;
    ss << "<!doctype html><html><head>"
    << "<title>hello websocket</title><body>"
    << "<h1>hello websocketpp</h1>"
    << "</body></head></html>";
    auto connection = server->get_con_from_hdl(handle);
    connection->set_body(ss.str());
    connection->set_status(websocketpp::http::status_code::ok);
}

int main() {
    WsServer server;
    server.set_access_channels(websocketpp::log::alevel::none); //不打印 websocketpp 日志
    server.init_asio(); //初始化底层网络框架

    server.set_open_handler(std::bind(&OnOpen, &server, std::placeholders::_1));
    server.set_close_handler(std::bind(&OnClose, &server, std::placeholders::_1));
    server.set_fail_handler(std::bind(&OnFail, &server, std::placeholders::_1));
    server.set_http_handler(std::bind(&OnHttp, &server, std::placeholders::_1));
    server.set_message_handler(std::bind(&OnMessage, &server, std::placeholders::_1, std::placeholders::_2));

    server.listen(9000); //监听9000端口
    server.start_accept(); // 开始接收tcp连接
    server.run(); //服务器启动

    return 0;
}
