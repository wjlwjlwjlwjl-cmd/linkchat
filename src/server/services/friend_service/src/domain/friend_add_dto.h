#pragma once

#include "common_dto.h"

struct FriendAddDTO: public CommonDTO {
    std::string notify_event_id;
};