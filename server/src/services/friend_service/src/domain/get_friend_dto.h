#pragma once

#include <string>
#include <vector>

#include "user.pb.h"

struct GetFriendDTO {
    bool success;
    std::string errmsg;
    std::vector<std::string> friend_ids; // 后续去调用user_service获取好友信息列表
};