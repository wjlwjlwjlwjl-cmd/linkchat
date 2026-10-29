// 负责维护服务-信道管理对象（Channels）的关系


#pragma once

#include "channels.h"
#include "adapter/logging/logger.h"

class SvcChannels {
public:
    //增加一个需要管理 channel 的服务
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
    svc_channels_[svc_name] = std::make_shared<Channels>();
}

void SvcChannels::AddNode(const std::string& svc_name, const std::string& addr) {
    std::unique_lock lock(mutex_);
    if (!svc_channels_.count(svc_name)) {
        LOG_ERROR("【增添】未找到 {} 服务管理对象（Channels）", svc_name);
        return;
    }
    svc_channels_[svc_name]->Insert(addr);
    LOG_INFO("{} 服务 {} 节点添加成功", svc_name, addr);
}

void SvcChannels::DelNode(const std::string& svc_name, const std::string& addr) {
    std::unique_lock lock(mutex_);
    if (!svc_channels_.count(svc_name)) {
        LOG_ERROR("【删除】未找到 {} 服务管理对象（Channels）", svc_name);
        return;
    }
    svc_channels_[svc_name]->Remove(addr);
    LOG_INFO("{} 服务 {} 节点删除成功", svc_name, addr);
}

ChannelPtr SvcChannels::GetChannel(const std::string& svc_name) {
    auto iter = svc_channels_.find(svc_name);
    return iter->second->SelectChannel();
}

std::optional<std::string> SvcChannels::GetAddr(const std::string& svc_name) {
    auto iter = svc_channels_.find(svc_name);
    return iter->second->SelectAddr();
}
