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
}