#pragma once

#include <string>
#include <vector>

//按照给定分隔符将字符串切分成若干部分
std::vector<std::string> split_str(const std::string& str, char spliter){
    std::vector<std::string> ret;
    size_t start = 0;
    size_t end = str.find(spliter);

    // 循环查找分隔符
    while (end != std::string::npos) {
        std::string item = str.substr(start, end - start);
        if(!item.empty())
            ret.push_back(item);
        start = end + 1;
        end = str.find(spliter, start);
    }
    
    // 处理最后一段（没有分隔符的部分，或者是末尾的残留）
    ret.push_back(str.substr(start));
    
    return ret;
}