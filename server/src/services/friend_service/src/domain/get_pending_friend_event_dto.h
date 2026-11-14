#pragma once

#include <vector>

#include "common_dto.h"
#include "user_info.h"

struct GetPendingFriendEventDTO: public CommonDTO {
    std::vector<std::string> event_ids;
};