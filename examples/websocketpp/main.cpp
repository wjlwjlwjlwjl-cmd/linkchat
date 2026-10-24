#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> ws_server;
typedef ws_server::message_ptr message_ptr;

void OnOpen(ws_server* svr, websocketpp::connection_hdl hdl) {
    std::cout << "ws_server open" << std::endl;
}

void OnClose(ws_server* svr, websocketpp::connection_hdl hdl) {
    std::cout << "ws_server close" << std::endl;
}

void OnFail(ws_server* svr, websocketpp::connection_hdl hdl) {
    std::cout << "ws_server fail" << std::endl;
}

void OnMessage(ws_server* svr, websocketpp::connection_hdl hdl, message_ptr msg) {
    std::cout << "ws_server get message" << msg->get_payload() << std::endl;
    svr->send(hdl, "this is a message returned from ws_server", websocketpp::frame::opcode::text); //通过四比特位的标签，来指示数据用途
}

//这里是设置了 http 方法，但是对于不同客户端处理方式，可能会出现 ws_server fail，但是回调正常的，消息传递也正常（点名 ApiFox）
void OnHttp(ws_server* svr, websocketpp::connection_hdl hdl) {
    std::cout << "ws_server get http" << std::endl;
    std::stringstream ss;
    ss << "<!doctype html><html><head>"
    << "<title>hello websocket</title><body>"
    << "<h1>hello websocketpp</h1>"
    << "</body></head></html>";
    auto con = svr->get_con_from_hdl(hdl);
    con->set_body(ss.str());
    con->set_status(websocketpp::http::status_code::ok);
}

int main() {
    ws_server svr;
    svr.set_access_channels(websocketpp::log::alevel::none); //不打印 websocketpp 日志
    svr.init_asio(); //初始化底层网络框架

    svr.set_open_handler(std::bind(&OnOpen, &svr, std::placeholders::_1));
    svr.set_close_handler(std::bind(&OnClose, &svr, std::placeholders::_1));
    svr.set_fail_handler(std::bind(&OnFail, &svr, std::placeholders::_1));
    svr.set_http_handler(std::bind(&OnHttp, &svr, std::placeholders::_1));
    svr.set_message_handler(std::bind(&OnMessage, &svr, std::placeholders::_1, std::placeholders::_2));

    svr.listen(9000); //监听9000端口
    svr.start_accept(); // 开始接收tcp连接
    svr.run(); //服务器启动

    return 0;
}