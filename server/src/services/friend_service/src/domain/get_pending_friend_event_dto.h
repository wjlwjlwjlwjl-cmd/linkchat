#pragma once

#include <vector>

#include "common_dto.h"

struct GetPendingFriendEventDTO: public CommonDTO {
    std::vector<std::string> event_ids;
};