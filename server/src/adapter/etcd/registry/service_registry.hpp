// 服务在这里将自己的信息：服务名称、节点地址交给 Etcd 服务器
// 使用 Etcd 而不是 Redis 存储服务节点信息键值对，原因主要是 Redis 在集群状态下，采取的是最终一致性；而 Etcd 通过 Raft 算法保证强一致性


#pragma once

#include <string>
#include <memory>
#include <etcd/KeepAlive.hpp>
#include <etcd/Client.hpp>

#include "../../../util/random_id.hpp"
#include "../../constants/etcd_constants.hpp"
#include "../../logging/logger.hpp"

class SvcRegistry{
public:
    SvcRegistry() = delete;
    SvcRegistry(const std::string& etcd_server_ip, 
        const std::string& svc_name,
        const std::string& svc_ip
    );

    //注册节点，节点名称（/服务/实例id：ip）
    bool Registry();
private:
    std::string _etcd_server_ip;
    std::string _instance_id;
    std::string _svc_name;
    std::string _svc_ip;
    std::shared_ptr<::etcd::KeepAlive> _keepalive;

    std::string MakeKey();

    bool WaitForConnection(::etcd::Client&);
};

SvcRegistry::SvcRegistry(const std::string& etcd_server_ip, 
    const std::string& svc_name,
    const std::string& svc_ip
)
    : _etcd_server_ip(etcd_server_ip)
    , _instance_id(random_util::getInstanceId())
    , _svc_name(svc_name)
    , _svc_ip(svc_ip)
{}

std::string SvcRegistry::MakeKey(){
    std::stringstream ss;
    ss << "/" << _svc_name << "/" << _instance_id;
    return ss.str();
}

bool SvcRegistry::WaitForConnection(::etcd::Client& client){
    int max_retry = EtcdConstants::MAX_RETRY;
    while(max_retry--){
        if(client.head().get().is_ok()){
            return true;
        }
    }
    return false;
}

bool SvcRegistry::Registry(){
    ::etcd::Client client(_etcd_server_ip);
    bool ret = WaitForConnection(client);
    if(!ret){
        ERR("连接 Etcd 服务器失败");
        return false;
    }

    auto lease = client.leasegrant(EtcdConstants::TTL).get();
    if(!lease.is_ok()){
        ERR("获取租约失败 {}", lease.error_message());
        return false;
    }
    int64_t lease_id = lease.value().lease();
    string svc_key = MakeKey();
    auto put_ret = client.put(svc_key, _svc_ip, lease_id).get();
    if(!put_ret.is_ok()){
        ERR("Etcd 注册节点 {}, ip: {} 失败：{}", svc_key, _svc_ip, put_ret.error_message());
        return false;
    }

    auto handler = [&](std::exception_ptr e){
        try{
            Registry();
        }
        catch(const std::exception_ptr& e){
            std::cout << "keepalive 失败"<< std::endl;
        }
    };

    _keepalive.reset(new ::etcd::KeepAlive(_etcd_server_ip, EtcdConstants::TTL, lease_id));

    INF("成功注册服务: svc_name {}, ip {}", _svc_name, _svc_ip);
    return true;
}