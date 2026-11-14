#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "user_info.h"

//消息类型
enum class MessageType {
    STRING,
    FILE,
};

struct StringMessageInfo {
    std::string content; //文字聊天内容，在 ES 中已经存储，从 ES 中获取
};

struct FileMessageInfo {
    std::optional<std::string> file_id; //文件id,客户端发送的时候不用设置
    std::optional<int64_t> file_size; //文件大小
    std::optional<std::string> file_name; //文件名称
    //文件数据，在ES中存储消息的时候只要id和元信息，不要文件数据, 服务端转发的时候也不需要填充
    std::optional<std::string> file_content; // 上传文件时，这里是文件base64编码的二进制内容
};

struct MessageContent {
    MessageType message_type = MessageType::STRING; //消息类型
    std::optional<StringMessageInfo> string_message; //文字消息
    std::optional<FileMessageInfo> file_message; //文件消息
};

//消息结构
struct MessageInfo {
    std::string message_id; //消息ID
    std::string chat_session_id; //消息所属聊天会话ID
    UserInfo sender; //消息发送者信息
    MessageContent message; //消息内容，包括类型及内容
};
