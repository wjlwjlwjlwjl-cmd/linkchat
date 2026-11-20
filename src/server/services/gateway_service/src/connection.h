// websocket 连接管理类

#include <mutex>
#include <set>
#include <unordered_map>
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

    // 添加一个连接（同一账号允许多端在线，例如网页 + 调试工具）
    void AddConnection(const std::string& user_id, connection_t conn){
        std::unique_lock<std::mutex> lock(mtx_);
        connections_[user_id].insert(conn);
        clients_[conn] = Client(user_id);
        LOG_INFO("add connection: {}", user_id);
    }

    // 获取该账号的全部连接（副本，调用方可在锁外发送）
    std::set<connection_t> GetConnections(const std::string& user_id){
        std::unique_lock<std::mutex> lock(mtx_);
        auto it = connections_.find(user_id);
        if(it == connections_.end()){
            LOG_ERROR("user's connection not found: {}", user_id);
            return {};
        }
        return it->second;
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

    // 删除指定连接（只移除断开的那一条，不影响同账号其他端）
    void RemoveConnection(const connection_t& conn){
        std::unique_lock<std::mutex> lock(mtx_);
        auto client_it = clients_.find(conn);
        if(client_it == clients_.end()){
            LOG_ERROR("client not found");
            return;
        }
        const std::string user_id = client_it->second.user_id;
        clients_.erase(client_it);
        auto it = connections_.find(user_id);
        if(it != connections_.end()){
            it->second.erase(conn);
            if(it->second.empty()) connections_.erase(it);
        }
        LOG_INFO("remove connection: {}", user_id);
    }
private:
    std::mutex mtx_;
    std::unordered_map<std::string, std::set<connection_t>> connections_; // uid -> 连接集合
    std::unordered_map<connection_t, Client> clients_; // 连接 -> 客户端信息
};