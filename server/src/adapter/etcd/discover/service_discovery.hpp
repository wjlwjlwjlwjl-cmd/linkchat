// 用来向 Etcd 服务器获取服务ip。服务键值对：/service-name/instance-id


#pragma once

#include <etcd/Client.hpp>
#include <etcd/Watcher.hpp>

#include "adapter/logging/logger.hpp"
#include "util/string_util.hpp"

class SvcDiscovery{
public:
    using ModCallback = std::function<void(const std::string&, const std::string&)>; //上线、下线回调处理，传入 svc_name、svc_ip 两个参数

    SvcDiscovery() = delete;
    SvcDiscovery(const std::string&, ModCallback&& svc_online, ModCallback&& svc_offline);

    void Watch(); //开始进行监听，监听 / 以获取所有服务的节点信息
private:
    std::string ParseKey(const std::string&); //解析出 svc_name

    void Callback(::etcd::Response const&); //监听回调函数

    std::string _etcd_server_ip;
    ModCallback _svc_online;
    ModCallback _svc_offline;
    std::shared_ptr<::etcd::Watcher> _watcher;
};

SvcDiscovery::SvcDiscovery(const std::string& etcd_server_ip
    , SvcDiscovery::ModCallback&& svc_online
    , SvcDiscovery::ModCallback&& svc_offline)
    : _etcd_server_ip(etcd_server_ip)
    , _svc_online(svc_online)
    , _svc_offline(svc_offline)
{}

std::string SvcDiscovery::ParseKey(const std::string& key){
    //  /svc_name/instance_id
    std::vector<std::string> items = split_str(key, '/');
    return items[0];
}

void SvcDiscovery::Callback(::etcd::Response const& response){
    if(!response.is_ok()){
        ERR("监听出错：{}", response.error_message());
    }
    for(auto ev: response.events()){
        std::string svc_name = ParseKey(ev.kv().key());
        if(ev.event_type() == ::etcd::Event::EventType::PUT){
            //节点上线
            INF("节点上线: Svc, {}; IP, {}", svc_name, ev.kv().as_string());
            _svc_online(svc_name, ev.kv().as_string()); //执行 online 回调
        }
        else if(ev.event_type() == ::etcd::Event::EventType::DELETE_){
            //节点下线
            INF("节点下线: Svc, {}; IP, {}", svc_name, ev.prev_kv().as_string());
            _svc_offline(svc_name, ev.prev_kv().as_string()); //执行offline 回调
        }
        else{
            //无效通知
            WRN("无效通知类型");
        }
    }
}

void SvcDiscovery::Watch(){
    ::etcd::Client client(_etcd_server_ip);
    auto resp = client.ls("/").get();
    if(resp.is_ok()){
        if(resp.keys().size() != 0){
            INF("开启监听时，已有以下节点");
            for(int i = 0; i < resp.keys().size(); i++){
                std::string key = resp.key(i);
                std::string svc_name = ParseKey(key);
                std::string ip = resp.value().as_string();

                INF("Svc, {}; IP, {}", svc_name, ip);
            }
        }
        else{
            INF("开启监听时，无可用节点");
        }
    }

    auto cb = std::bind(&SvcDiscovery::Callback, this, std::placeholders::_1);

    std::string root_key = "/";
    _watcher = std::make_shared<::etcd::Watcher>(client, root_key, cb, true);
    _watcher->Wait([&](bool canceled){
        if(canceled){
            return;
        }
        Watch(); //继续监听
    });
}