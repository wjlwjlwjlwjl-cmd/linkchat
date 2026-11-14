#pragma once

#include <vector>

#include "common_dto.h"

struct GetChatSessionMemberDTO: public CommonDTO {
    std::vector<std::string> members_id; // 这里只检索出来用户id，实际的 user_info 获取依然交给 service 层
};