// 负责维护服务-信道管理对象（Channels）的关系


#pragma once

#include "channels.hpp"
#include "adapter/logging/logger.hpp"

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
    std::mutex _mtx;
    std::unordered_map<std::string, Channels::ptr> _svc_channels;
};

void SvcChannels::AddWatch(const std::string& svc_name) {
    std::unique_lock lock(_mtx);
    _svc_channels[svc_name] = std::make_shared<Channels>();
}

void SvcChannels::AddNode(const std::string& svc_name, const std::string& addr) {
    std::unique_lock lock(_mtx);
    if (!_svc_channels.count(svc_name)) {
        ERR("【增添】未找到 {} 服务管理对象（Channels）", svc_name);
        return;
    }
    _svc_channels[svc_name]->Insert(addr);
    INF("{} 服务 {} 节点添加成功", svc_name, addr);
}

void SvcChannels::DelNode(const std::string& svc_name, const std::string& addr) {
    std::unique_lock lock(_mtx);
    if (!_svc_channels.count(svc_name)) {
        ERR("【删除】未找到 {} 服务管理对象（Channels）", svc_name);
        return;
    }
    _svc_channels[svc_name]->Remove(addr);
    INF("{} 服务 {} 节点删除成功", svc_name, addr);
}

ChannelPtr SvcChannels::GetChannel(const std::string& svc_name) {
    auto it = _svc_channels.find(svc_name);
    return it->second->SelectChannel();
}

std::optional<std::string> SvcChannels::GetAddr(const std::string& svc_name) {
    auto it = _svc_channels.find(svc_name);
    return it->second->SelectAddr();
}