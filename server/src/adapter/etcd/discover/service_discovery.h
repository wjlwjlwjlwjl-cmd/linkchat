// 用来向 Etcd 服务器获取服务ip。服务键值对：/service-name/instance-id


#pragma once

#include <etcd/Client.hpp>
#include <etcd/Watcher.hpp>

#include "adapter/logging/logger.h"
#include "util/string_util.h"

class SvcDiscovery {
public:
    using ModCallback = std::function<void(const std::string&, const std::string&)>; //上线、下线回调处理，传入 svc_name、svc_ip 两个参数

    SvcDiscovery() = delete;
    SvcDiscovery(const std::string& etcd_server_ip, ModCallback&& svc_online, ModCallback&& svc_offline);

    void Watch(); //开始进行监听，监听 / 以获取所有服务的节点信息
private:
    std::string ParseKey(const std::string& key); //解析出 svc_name

    void Callback(::etcd::Response const& response); //监听回调函数

    std::string etcd_server_ip_;
    ModCallback svc_online_;
    ModCallback svc_offline_;
    std::shared_ptr<::etcd::Watcher> watcher_;
};

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
    if (!response.is_ok()) {
        LOG_ERROR("监听出错：{}", response.error_message());
    }
    for (auto event : response.events()) {
        std::string svc_name = ParseKey(event.kv().key());
        if (event.event_type() == ::etcd::Event::EventType::PUT) {
            //节点上线
            LOG_INFO("节点上线: Svc, {}; IP, {}", svc_name, event.kv().as_string());
            svc_online_(svc_name, event.kv().as_string()); //执行 online 回调
        }
        else if (event.event_type() == ::etcd::Event::EventType::DELETE_) {
            //节点下线
            LOG_INFO("节点下线: Svc, {}; IP, {}", svc_name, event.prev_kv().as_string());
            svc_offline_(svc_name, event.prev_kv().as_string()); //执行offline 回调
        }
        else {
            //无效通知
            LOG_WARN("无效通知类型");
        }
    }
}

inline void SvcDiscovery::Watch() {
    ::etcd::Client client(etcd_server_ip_);
    auto resp = client.ls("/").get();
    if (resp.is_ok()) {
        if (resp.keys().size() != 0) {
            LOG_INFO("开启监听时，已有以下节点");
            for (int i = 0; i < resp.keys().size(); i++) {
                std::string key = resp.key(i);
                std::string svc_name = ParseKey(key);
                std::string ip = resp.value().as_string();

                LOG_INFO("Svc, {}; IP, {}", svc_name, ip);
            }
        }
        else {
            LOG_INFO("开启监听时，无可用节点");
        }
    }

    auto callback = std::bind(&SvcDiscovery::Callback, this, std::placeholders::_1);

    std::string root_key = "/";
    watcher_ = std::make_shared<::etcd::Watcher>(client, root_key, callback, true);
    watcher_->Wait([&](bool canceled) {
        if (canceled) {
            return;
        }
        Watch(); //继续监听
    });
}
