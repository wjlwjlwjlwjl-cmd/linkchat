// 用来向 Etcd 服务器获取服务ip。服务键值对：/service-name/instance-id


#pragma once

#include <etcd/Client.hpp>
#include <etcd/Watcher.hpp>
#include <mutex>

#include "logging/logger.h"
#include "rpc/channels.h"
#include "string_util.h"
#include "rpc/svc_channels.h"

class SvcDiscovery {
public:
    using Ptr = std::shared_ptr<SvcDiscovery>;
    using ModCallback = std::function<void(const std::string&, const std::string&)>; //上线、下线回调处理，传入 svc_name、svc_ip 两个参数

    SvcDiscovery() = delete;
    SvcDiscovery(const std::string& etcd_server_ip, ModCallback&& svc_online, ModCallback&& svc_offline);

    void Watch(); //开始进行监听，监听 / 以获取所有服务的节点信息

    ChannelPtr GetSvcChannel(const std::string& svc_name);
    std::string GetSvcIp(const std::string& svc_name);
private:
    std::string ParseKey(const std::string& key); //解析出 svc_name

    void Callback(::etcd::Response const& response); //监听回调函数

    std::string etcd_server_ip_;
    ModCallback svc_online_; //自定义上线回调
    ModCallback svc_offline_; //自定义下线回调
    std::shared_ptr<::etcd::Watcher> watcher_;

    std::mutex mtx_;
    SvcChannels svc_channels_; // svc_name <-> svc_channel
};

inline std::string SvcDiscovery::GetSvcIp(const std::string& svc_name){
    auto ret = svc_channels_.GetAddr(svc_name);
    if(ret.has_value()){
        return ret.value();
    }
    else{
        return "";
    }
}

inline ChannelPtr SvcDiscovery::GetSvcChannel(const std::string& svc_name){
    return svc_channels_.GetChannel(svc_name);
}

inline SvcDiscovery::SvcDiscovery(const std::string& etcd_server_ip
    , SvcDiscovery::ModCallback&& svc_online
    , SvcDiscovery::ModCallback&& svc_offline)
    : etcd_server_ip_(etcd_server_ip)
    , svc_online_(svc_online)
    , svc_offline_(svc_offline)
{}

inline std::string SvcDiscovery::ParseKey(const std::string& key) {
    //  /svc_name/instance_id
    std::vector<std::string> items = SplitStr(key, '/');
    return items[0];
}

inline void SvcDiscovery::Callback(::etcd::Response const& response) {
    //在监听中，进行服务ip列表的修改
    if (!response.is_ok()) {
        LOG_ERROR("Watch failed: {}", response.error_message());
    }
    for (auto event : response.events()) {
        std::string svc_name = ParseKey(event.kv().key());
        if (event.event_type() == ::etcd::Event::EventType::PUT) {
            //节点上线
            LOG_INFO("Service online, svc: {}, ip: {}", svc_name, event.kv().as_string());
            svc_channels_.AddNode(svc_name, event.kv().as_string());
            svc_online_(svc_name, event.kv().as_string()); //执行 online 回调
        }
        else if (event.event_type() == ::etcd::Event::EventType::DELETE_) {
            //节点下线
            LOG_INFO("Service offline, svc: {}, ip: {}", svc_name, event.prev_kv().as_string());
            svc_channels_.DelNode(svc_name, event.prev_kv().as_string());
            svc_offline_(svc_name, event.prev_kv().as_string()); //执行offline 回调
        }
        else {
            //无效通知
            LOG_WARN("Unsupported event type");
        }
    }
}

inline void SvcDiscovery::Watch() {
    ::etcd::Client client(etcd_server_ip_);
    auto resp = client.ls("/").get();
    if (resp.is_ok()) {
        LOG_INFO("start to get all of the svc_nodes...");
        // 在开始监听时，进行一次全量的服务信息获取
        if (resp.keys().size() != 0) {
            for (int i = 0; i < resp.keys().size(); i++) {
                std::string key = resp.key(i);
                std::string svc_name = ParseKey(key);
                std::string ip = resp.value(i).as_string();

                svc_channels_.AddWatch(svc_name);
                svc_channels_.AddNode(svc_name, ip);
                LOG_INFO("svc_name: {}, svc_ip: {}", svc_name, ip);
            }
        }
        else {
            LOG_INFO("No available node when starting watch");
        }
    }

    auto callback = std::bind(&SvcDiscovery::Callback, this, std::placeholders::_1);

    std::string root_key = "/";
    watcher_ = std::make_shared<::etcd::Watcher>(client, root_key, callback, true);
    watcher_->Wait([&](bool canceled) {
        if (canceled) {
            return;
        }
        LOG_INFO("sleep 3 seconds to rewatch");
        std::this_thread::sleep_for(std::chrono::seconds(3));
        Watch(); //继续监听
    });
}
