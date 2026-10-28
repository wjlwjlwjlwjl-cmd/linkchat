//负责管理一个服务的所有 Channel（brpc client 通过 brpc::Channel 和 brpc server 进行通信）


#pragma once
#include <brpc/channel.h>
#include <optional>

#include "constants/rpc_constants.hpp"
#include "adapter/logging/logger.hpp"

using ChannelPtr = std::shared_ptr<brpc::Channel>;

class Channels {
public:
    using ptr = std::shared_ptr<Channels>;

    //新增节点
    inline void Insert(const std::string& addr);

    //删除节点
    inline bool Remove(const std::string& addr);

    //获取节点信道
    inline ChannelPtr SelectChannel();

    //获取节点地址
    inline std::optional<std::string> SelectAddr();
private:
    std::mutex _mtx;
    int _idx = 0;
    std::string _service_name; //这个信道管理对象，对应的服务名称
    std::vector<std::pair<std::string, ChannelPtr>> _channels; //节点ip：brpc::channel
};

void Channels::Insert(const std::string& addr) {
    //在这里建立这个节点的信道
    std::unique_lock<std::mutex> lock(_mtx);

    brpc::ChannelOptions channel_options;
    channel_options.connect_timeout_ms = RpcConstants::CONNECT_TIMEOUT_MS;
    channel_options.max_retry = RpcConstants::MAX_RETRY;
    channel_options.timeout_ms = RpcConstants::TIMEOUT_MS;
    channel_options.protocol = RpcConstants::RPC_PROTOCOL;

    ChannelPtr channel = std::make_shared<brpc::Channel>();
    if (channel->Init(addr.c_str(), &channel_options) != 0) {
        ERR("Failed to initialize channel to {}", addr);
        return;
    }
    _channels.push_back(std::make_pair(addr, channel));
}

bool Channels::Remove(const std::string& addr) {
    std::unique_lock<std::mutex> lock(_mtx);

    for (auto e = _channels.begin(); e != _channels.end(); ++e) {
        if (e->first == addr) {
            _channels.erase(e);
            return true;
        }
    }
    return false;
}

ChannelPtr Channels::SelectChannel() {
    std::unique_lock<std::mutex> lock(_mtx);
    if (_channels.empty()) {
        return nullptr;
    }
    auto ret_pair = _channels[_idx++];
    _idx %= _channels.size();
    return ret_pair.second;
}

inline std::optional<std::string> Channels::SelectAddr() {
    std::unique_lock<std::mutex> lock(_mtx);
    if (_channels.empty()) {
        return nullptr;
    }
    auto ret_pair = _channels[_idx++];
    _idx %= _channels.size();
    return ret_pair.first;
}