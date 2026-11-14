#pragma once

#include <string>
#include <vector>

#include "user.pb.h"

struct GetFriendDTO {
    bool success;
    std::string errmsg;
    std::vector<::linkchat::UserInfo> user_info_list;
};