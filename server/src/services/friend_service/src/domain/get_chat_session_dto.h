#pragma once

#include <optional>

#include "common_dto.h"

struct ChatSessionInfo{
    std::optional<std::string> single_chat_friend_id;
    std::string chat_session_id;
    std::string chat_session_name;
};

struct GetChatSessionDTO: public CommonDTO{
};