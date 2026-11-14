// 各个微服务的名称，用来注册和服务发现


#pragma once

namespace service_name{
    inline constexpr const char* file_service = "FILE_SERVICE";
    inline constexpr const char* message_storage_service = "MESSAGE_SERVICE";
    inline constexpr const char* user_service = "USER_SERVICE";
    inline constexpr const char* transmite_service = "TRANSMITE_SERVICE";
}

namespace service_prefix{
    inline constexpr const char* user_avatar_prefix = "avatar_";
    inline constexpr const char* friend_apply_prefix = "friend_apply_";
    inline constexpr const char* chat_session_single_prefix = "chat_session_s_";
    inline constexpr const char* chat_session_group_prefix = "chat_session_g_";
}