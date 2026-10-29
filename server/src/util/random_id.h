// 随机数工具类


#pragma once

#include <iomanip>
#include <random>
#include <string>
#include <iostream>

namespace random_util {
    inline std::string GetInstanceId() {
        std::random_device rd;
        std::mt19937 mt(rd()); //设备随机数
        std::uniform_int_distribution<uint32_t> dis;

        uint32_t random_num = dis(mt);

        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(8) << random_num;
        return ss.str();
    }
}
