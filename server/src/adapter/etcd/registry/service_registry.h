// 服务在这里将自己的信息：服务名称、节点地址交给 Etcd 服务器
// 使用 Etcd 而不是 Redis 存储服务节点信息键值对，原因主要是 Redis 在集群状态下，采取的是最终一致性；而 Etcd 通过 Raft 算法保证强一致性


#pragma once

#include <string>
#include <memory>
#include <etcd/KeepAlive.hpp>
#include <etcd/Client.hpp>

#include "util/random_id.h"
#include "constants/etcd_constants.h"
#include "adapter/logging/logger.h"

class SvcRegistry {
public:
    SvcRegistry() = delete;
    SvcRegistry(const std::string& etcd_server_ip,
        const std::string& svc_name,
        const std::string& svc_ip
    );

    //注册节点，节点名称（/服务/实例id：ip）
    bool Registry();
private:
    std::string etcd_server_ip_;
    std::string instance_id_;
    std::string svc_name_;
    std::string svc_ip_;
    std::shared_ptr<::etcd::KeepAlive> keepalive_;

    std::string MakeKey();

    bool WaitForConnection(::etcd::Client& client);
};

inline SvcRegistry::SvcRegistry(const std::string& etcd_server_ip,
    const std::string& svc_name,
    const std::string& svc_ip
)
    : etcd_server_ip_(etcd_server_ip)
    , instance_id_(random_util::GetInstanceId())
    , svc_name_(svc_name)
    , svc_ip_(svc_ip)
{}

inline std::string SvcRegistry::MakeKey() {
    std::stringstream ss;
    ss << "/" << svc_name_ << "/" << instance_id_;
    return ss.str();
}

inline bool SvcRegistry::WaitForConnection(::etcd::Client& client) {
    int max_retry = etcd_constants::kMaxRetry;
    while (max_retry--) {
        if (client.head().get().is_ok()) {
            return true;
        }
    }
    return false;
}

inline bool SvcRegistry::Registry() {
    ::etcd::Client client(etcd_server_ip_);
    bool ret = WaitForConnection(client);
    if (!ret) {
        LOG_ERROR("连接 Etcd 服务器失败");
        return false;
    }

    auto lease = client.leasegrant(etcd_constants::kTtl).get();
    if (!lease.is_ok()) {
        LOG_ERROR("获取租约失败 {}", lease.error_message());
        return false;
    }
    int64_t lease_id = lease.value().lease();
    std::string svc_key = MakeKey();
    auto put_ret = client.put(svc_key, svc_ip_, lease_id).get();
    if (!put_ret.is_ok()) {
        LOG_ERROR("Etcd 注册节点 {}, ip: {} 失败：{}", svc_key, svc_ip_, put_ret.error_message());
        return false;
    }

    auto handler = [&](std::exception_ptr e) {
        try {
            Registry();
        }
        catch (const std::exception_ptr& e) {
            std::cout << "keepalive 失败" << std::endl;
        }
    };

    keepalive_.reset(new ::etcd::KeepAlive(etcd_server_ip_, etcd_constants::kTtl, lease_id));

    LOG_INFO("成功注册服务: svc_name {}, ip {}", svc_name_, svc_ip_);
    return true;
}
