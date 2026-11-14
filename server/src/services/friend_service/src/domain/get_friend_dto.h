#pragma once

#include <vector>

#include "common_dto.h"

struct GetFriendDTO : public CommonDTO {
    std::vector<std::string> friend_ids; // 后续去调用user_service获取好友信息列表
};