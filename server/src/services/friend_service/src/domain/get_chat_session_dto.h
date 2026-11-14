#pragma once

#include <optional>
#include <string>

#include "common_dto.h"
#include "message_info.h"

//聊天会话信息
struct ChatSessionInfo {
    //群聊会话不需要设置，单聊会话设置为对方用户ID
    std::optional<std::string> single_chat_friend_id;
    std::string chat_session_id; //会话ID
    std::string chat_session_name; //会话名称
    //会话上一条消息，新建的会话没有最新消息
    std::optional<MessageInfo> prev_message;
    //会话头像 file_id --群聊会话不需要，直接由前端固定渲染，单聊就是对方的头像
    std::string avatar;
};

struct GetChatSessionDTO: public CommonDTO{
};
