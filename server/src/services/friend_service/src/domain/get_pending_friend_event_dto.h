#pragma once

#include <vector>

#include "common_dto.h"

struct GetPendingFriendEventDTO: public CommonDTO {
    // 在 dao 层中筛选出来好友申请事件 id，以及被申请人id
    std::vector<std::pair<std::string, std::string>> ev_resp_list;
};