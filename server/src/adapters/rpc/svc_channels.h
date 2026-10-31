// 负责维护服务-信道管理对象（Channels）的关系


#pragma once

#include "channels.h"
#include "logging/logger.h"

class SvcChannels {
public:
    //增加一个需要管理 channel 的服务（每次都要调用，如果已添加会跳过）
    inline void AddWatch(const std::string& svc_name);

    //给服务增添一个节点
    inline void AddNode(const std::string& svc_name, const std::string& addr);

    //删除一个服务的一个节点
    inline void DelNode(const std::string& svc_name, const std::string& addr);

    //获取一个节点信道
    inline ChannelPtr GetChannel(const std::string& svc_name);

    //获取节点地址
    inline std::optional<std::string> GetAddr(const std::string& svc_name);
private:
    std::mutex mutex_;
    std::unordered_map<std::string, Channels::Ptr> svc_channels_;
};

void SvcChannels::AddWatch(const std::string& svc_name) {
    std::unique_lock lock(mutex_);
    if(svc_channels_.count(svc_name)){
        return;
    }
    svc_channels_[svc_name] = std::make_shared<Channels>();
}

void SvcChannels::AddNode(const std::string& svc_name, const std::string& addr) {
    std::unique_lock lock(mutex_);
    if (!svc_channels_.count(svc_name)) {
        LOG_ERROR("Failed to add node, Channels object of service {} not found", svc_name);
        return;
    }
    svc_channels_[svc_name]->Insert(addr);
    LOG_INFO("Added node {} to service {}", addr, svc_name);
}

void SvcChannels::DelNode(const std::string& svc_name, const std::string& addr) {
    std::unique_lock lock(mutex_);
    if (!svc_channels_.count(svc_name)) {
        LOG_ERROR("Failed to remove node, Channels object of service {} not found", svc_name);
        return;
    }
    svc_channels_[svc_name]->Remove(addr);
    LOG_INFO("Removed node {} from service {}", addr, svc_name);
}

ChannelPtr SvcChannels::GetChannel(const std::string& svc_name) {
    std::unique_lock lock(mutex_);
    auto iter = svc_channels_.find(svc_name);
    if(iter == svc_channels_.end() || iter->second == nullptr){
        return nullptr;
    }
    return iter->second->SelectChannel();
}

std::optional<std::string> SvcChannels::GetAddr(const std::string& svc_name) {
    auto iter = svc_channels_.find(svc_name);
    if(iter != svc_channels_.end() && iter->second != nullptr){
        return iter->second->SelectAddr();
    }
    return std::nullopt;
}
