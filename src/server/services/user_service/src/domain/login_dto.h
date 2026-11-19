#pragma once

#include <string>

struct LoginDTO{
public:
    bool success;
    std::string errmsg;
    std::string token;
};