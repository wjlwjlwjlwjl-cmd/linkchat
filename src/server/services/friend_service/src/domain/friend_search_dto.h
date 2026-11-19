#pragma once

#include <string>
#include <vector>
#include "common_dto.h"

struct FriendSearchDTO: public CommonDTO{
    std::vector<std::string> users_id; // 被检索出来的用户id
};