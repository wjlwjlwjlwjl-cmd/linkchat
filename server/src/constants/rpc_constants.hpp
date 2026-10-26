//brpc 相关配置常量


#pragma once

#include <string>

namespace RpcConstants {
    //C++17 后，加了 inline，类内如 string （非基本类型）声明即定义，不加需要类外定义，否则链接报错
    inline const static std::string RPC_PROTOCOL = "baidu_std";
    const static int MAX_RETRY = 10;
    const static int TIMEOUT_MS = 10000;
    const static int CONNECT_TIMEOUT_MS = 10000;
};