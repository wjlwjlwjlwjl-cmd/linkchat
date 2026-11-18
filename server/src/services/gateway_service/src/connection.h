// websocket 连接管理类

#include <mutex>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "logging/logger.h"

using server_t = websocketpp::server<websocketpp::config::asio>;
using connection_t = server_t::connection_ptr; // 连接类型

class Connection{
public:
    using ptr = std::shared_ptr<Connection>;
    struct Client{
        Client() = default;
        Client(const std::string& user_id)
            : user_id(user_id) {}

        std::string user_id; // 用户 id

        operator std::string() const{
            return "user_id: " + user_id;
        }
    };

    Connection() = default;
    ~Connection() = default;

    // 添加一个连接
    void AddConnection(const std::string& user_id, connection_t conn){
        std::unique_lock<std::mutex> lock(mtx_);
        connections_[user_id] = conn;
        clients_[conn] = Client(user_id);
        LOG_INFO("add connection: {}", user_id);
    }

    // 获取一个连接
    connection_t GetConnection(const std::string& user_id){
        std::unique_lock<std::mutex> lock(mtx_);
        if(connections_.find(user_id) == connections_.end()){
            LOG_ERROR("user's connection not found: {}", user_id);
            return nullptr;
        }
        return connections_[user_id];
    }

    // 获取一个客户端信息
    bool GetClient(const connection_t conn, std::string& uid, std::string& token){
        std::unique_lock<std::mutex> lock(mtx_);
        if(clients_.find(conn) == clients_.end()){
            return false;
        }
        uid = clients_[conn].user_id;
        return true;
    }

    // 删除一个连接
    void RemoveConnection(const std::string& user_id){
        std::unique_lock<std::mutex> lock(mtx_);
        auto conn = connections_[user_id];
        if(!conn){
            LOG_ERROR("user's connection not found: {}", user_id);
            return;
        }
        connections_.erase(user_id);
        if(clients_.erase(conn) == 0){
            LOG_ERROR("client not found: {}", user_id);
            return;
        }
        LOG_INFO("remove connection: {}", user_id);
    }
private:
    std::mutex mtx_;
    std::unordered_map<std::string, connection_t> connections_; // uid -> 连接
    std::unordered_map<connection_t, Client> clients_; // 连接 -> 客户端信息
};