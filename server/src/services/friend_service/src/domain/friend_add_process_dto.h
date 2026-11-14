#pragma once
#include "common_dto.h"
#include <string>

class FriendAddProcessDTO : public CommonDTO {
public:
    // 同意后，新创建的会话 id
    std::string new_session_id;
};