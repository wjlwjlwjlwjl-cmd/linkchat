// 随机数工具类


#pragma once

#include <iomanip>
#include <random>
#include <string>
#include <iostream>

namespace random_util {
    /// @brief 生成实例ID
    /// @return 8位随机数
    inline std::string GetInstanceId() {
        std::random_device rd;
        std::mt19937 mt(rd()); //设备随机数
        std::uniform_int_distribution<uint32_t> dis;

        uint32_t random_num = dis(mt);

        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(8) << random_num;
        return ss.str();
    }

    /// @brief 生成验证码
    /// @return 6位验证码
    inline std::string GetVerifyCode() {
        // 线程局部复用随机引擎，避免每次调用都构造 random_device / mt19937
        static thread_local std::mt19937 mt(std::random_device{}());
        std::uniform_int_distribution<uint32_t> dis(0, 999999);

        uint32_t code = dis(mt);

        std::string s = std::to_string(code);
        if (s.size() < 6) {
            s.insert(0, 6 - s.size(), '0');
        }
        return s;
    }
}
