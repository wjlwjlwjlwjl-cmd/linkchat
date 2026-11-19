//负责管理一个服务的所有 Channel（brpc client 通过 brpc::Channel 和 brpc server 进行通信）


#pragma once
#include <brpc/channel.h>
#include <optional>

#include "rpc_constants.h"
#include "logging/logger.h"

using ChannelPtr = std::shared_ptr<brpc::Channel>;

class Channels {
public:
    using Ptr = std::shared_ptr<Channels>;

    Channels() = delete;

    Channels(const std::string& svc_name);

    //新增节点
    inline void Insert(const std::string& addr);

    //删除节点
    inline bool Remove(const std::string& addr);

    //获取节点信道和地址
    inline ChannelPtr SelectChannel(std::string& ip);
private:
    std::mutex mutex_;
    int index_ = 0;
    std::string service_name_; //这个信道管理对象，对应的服务名称
    std::vector<std::pair<std::string, ChannelPtr>> channels_; //节点ip：brpc::channel
};

inline Channels::Channels(const std::string& svc_name)
    : service_name_(svc_name)
{}

inline void Channels::Insert(const std::string& addr) {
    //在这里建立这个节点的信道
    brpc::ChannelOptions channel_options;
    channel_options.connect_timeout_ms = rpc_constants::kConnectTimeoutMs;
    channel_options.max_retry = rpc_constants::kMaxRetry;
    channel_options.timeout_ms = rpc_constants::kTimeoutMs;
    channel_options.protocol = rpc_constants::kRpcProtocol;

    ChannelPtr channel = std::make_shared<brpc::Channel>();
    if (channel->Init(addr.c_str(), &channel_options) != 0) {
        LOG_ERROR("Failed to initialize channel to {}", addr);
        return;
    }
    {
        std::unique_lock<std::mutex> lock(mutex_);
        channels_.push_back(std::make_pair(addr, channel));
    }
}

inline bool Channels::Remove(const std::string& addr) {
    std::unique_lock<std::mutex> lock(mutex_);

    for (auto iter = channels_.begin(); iter != channels_.end(); ++iter) {
        if (iter->first == addr) {
            channels_.erase(iter);
            return true;
        }
    }
    return false;
}

inline ChannelPtr Channels::SelectChannel(std::string& ip) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (channels_.empty()) {
        LOG_WARN("no nodes of {} available", service_name_);
        return nullptr;
    }
    auto ret_pair = channels_[index_++];
    index_ = (index_ + 1) % channels_.size();
    ip = ret_pair.first;
    return ret_pair.second;
}