#pragma once

#include <string>
#include <vector>

#include "common_dto.h"
#include "message_info.h"

// 新消息请求
struct NewMessageDTO {
    std::string token; // 标识发送者身份
    std::string chat_session_id; //聊天会话ID -- 标识了当前消息属于哪个会话，应该转发给谁
    MessageContent message; // 消息内容--消息类型+内容
};

// 消息转发目标
struct GetTransmitTargetDTO : public CommonDTO {
    MessageInfo message; // 组织好的消息结构，里面包含消息内容，以及这个消息对应的会话 id
    std::vector<std::string> target_id_list; //消息的转发目标列表，网关通过 user_id 标识 ws 长连接
};
