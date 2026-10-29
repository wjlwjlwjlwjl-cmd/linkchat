#pragma once

#include <string>
#include <vector>

//按照给定分隔符将字符串切分成若干部分
inline std::vector<std::string> SplitStr(const std::string& str, char separator) {
    std::vector<std::string> ret;
    size_t start = 0;
    size_t end = str.find(separator);

    // 循环查找分隔符
    while (end != std::string::npos) {
        std::string item = str.substr(start, end - start);
        if (!item.empty())
            ret.push_back(item);
        start = end + 1;
        end = str.find(separator, start);
    }

    // 处理最后一段（没有分隔符的部分，或者是末尾的残留）
    ret.push_back(str.substr(start));

    return ret;
}
