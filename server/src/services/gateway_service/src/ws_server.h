// 启动 websocket 服务器
// websocket 服务器不处理客户端自身通知，只完成对客户端的消息推送
// 各种请求通过 gateway 提供的 http 方法完成

#include <functional>
#include <websocketpp/close.hpp>

#include "connection.h"
#include "jwt_util.h"
#include "error_msg.h"
#include "verify_token.h"
#include "redis/redis_factory.h"

using message_t = server_t::message_ptr;

class WebSocketServer {
public:
    WebSocketServer(int ws_port, redis_factory::RedisConnectionInfo& redis_connect_info){
        ws_port_ = ws_port;
        redis_template_ = redis_factory::RedisFactory::Create(redis_connect_info);
    }

    void run(){
        server_.set_access_channels(websocketpp::log::alevel::none); // 关闭日志打印
        server_.init_asio(); // 初始化底层网络框架

        // 设置操作回调
        server_.set_open_handler(std::bind(&WebSocketServer::OnOpen, this, &server_, std::placeholders::_1));
        server_.set_close_handler(std::bind(&WebSocketServer::OnClose, this, &server_, std::placeholders::_1));
        server_.set_fail_handler(std::bind(&WebSocketServer::OnFail, this, &server_, std::placeholders::_1));
        server_.set_message_handler(std::bind(&WebSocketServer::OnMessage, this, &server_, std::placeholders::_1, std::placeholders::_2));

        // 启动 websocket 服务器
        ws_server_thread_ = std::thread([this](){
            LOG_INFO("WebSocket server started on port: {}", ws_port_);
            server_.listen(ws_port_);
            server_.start_accept();
            server_.run();
        });
    }

    void stop(){
        server_.stop();
        if(ws_server_thread_.joinable()){
            ws_server_thread_.join();
        }
        LOG_INFO("websocket server stopped");
    }

private:
    bool get_token(const std::string& raw, std::string& token){
        if(raw.empty()) return false;
        size_t pos = raw.find("token=");
        if (pos != std::string::npos) {
            token = raw.substr(pos + 6); // "token=" 长度是6
            size_t amp = token.find('&');
            if (amp != std::string::npos) {
                token = token.substr(0, amp);
            }
        }
        if(pos == std::string::npos) return false; // 没有或者不是 token= 开头
        return true;
    }

    // 连接打开
    void OnOpen(server_t *server, websocketpp::connection_hdl handle) {
        websocketpp::lib::error_code ec;
        auto conn = server_.get_con_from_hdl(handle, ec);
        if(ec && !conn){
            LOG_WARN("get_con_from_hdl failed: {}", ec.message());
            server->close(handle, websocketpp::close::status::policy_violation, errmsg::gateway::kWebSocketFailed);
            return;
        }
        auto uri = conn->get_uri();
        std::string raw = uri->get_query();
        std::string token;
        if(!get_token(raw, token)){
            server->close(handle, websocketpp::close::status::policy_violation, errmsg::kTokenInvalid);
            return;
        }
        if(!VerifyToken(token, redis_template_)){
            server->close(handle, websocketpp::close::status::policy_violation, errmsg::kTokenInvalid);
            return;
        }

        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            server->close(handle, websocketpp::close::status::policy_violation, errmsg::kTokenInvalid);
            return;
        }

        // 连接成功，将 user_id 放到连接管理类中
        connection_.AddConnection(user_id, conn);
        LOG_DEBUG("add connection: {}", user_id);

        return;
    }

    // 连接关闭
    void OnClose(server_t *server, websocketpp::connection_hdl handle) {
        // 我们前面就维护了 conn -> user_id + token 的映射
        auto conn = server_.get_con_from_hdl(handle);
        std::string user_id, token;
        if(!connection_.GetClient(conn, user_id, token)){
            return;
        }
        connection_.RemoveConnection(user_id);
        LOG_DEBUG("remove connection: {}", user_id);
    }

    // 连接断开
    void OnFail(server_t *server, websocketpp::connection_hdl handle) {
        auto conn = server_.get_con_from_hdl(handle);
        std::string user_id, token;
        if(!connection_.GetClient(conn, user_id, token)){
            LOG_ERROR("client not found (OnFail)");
            return;
        }
        connection_.RemoveConnection(user_id);
        LOG_DEBUG("remove connection: {}", user_id);
    }

    // 收到消息
    void OnMessage(server_t *server, websocketpp::connection_hdl handle,
                   message_t msg) {
        // websocket 服务器的在这里只做服务端到客户端的消息推送，不做逻辑，但是为了防止客户端因没有设置回调时发消息断开，这里还是注册
    }

    int ws_port_;
    std::shared_ptr<::sw::redis::Redis> redis_template_;
    Connection connection_; // 连接管理类
    server_t server_;
    std::thread ws_server_thread_;
};