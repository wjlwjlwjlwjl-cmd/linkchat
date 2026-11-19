#pragma once

#include <string>

struct UserInfo {
    std::string user_id; //用户ID
    std::string nickname; //昵称
    std::string description; //个人签名/描述
    std::string email; //绑定邮箱
    std::string avatar; //头像照片，这里存放预签名 url
};
