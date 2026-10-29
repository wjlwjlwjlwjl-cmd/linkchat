#include <string>
#include <fstream>
#include <iostream>

#include "adapter/logging/logger.hpp"

namespace file{
    std::string ReadFileToString(const std::string& path){
        std::ifstream in(path, std::ios::binary | std::ios::ate);
        if(!in){
            ERR("failed to open file {}", path);
        }
        int file_size = in.tellg();
        in.seekg(std::ios::beg);

        std::string content;
        content.resize(file_size);

        if(in.read(content.data(), content.size())){
            return content;
        }
        return "";
    }
}