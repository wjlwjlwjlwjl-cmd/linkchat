//brpc 相关配置常量


#pragma once

#include <string>

namespace rpc_constants {
    inline constexpr char kRpcProtocol[] = "baidu_std";
    inline constexpr int kMaxRetry = 10;
    inline constexpr int kTimeoutMs = 10000;
    inline constexpr int kConnectTimeoutMs = 10000;
};
