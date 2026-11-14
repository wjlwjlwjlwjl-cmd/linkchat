#pragma once
// 这里提供的都是秒为单位的过期时间，set、expire 都直接使用秒即可


#include <string>

namespace redis_constants{
    static constexpr const char* email_verify_code_key = "email_verify_code:";
    static constexpr int email_verify_code_expire = 5 * 60; // 5 minutes, seconds

    static constexpr const char* token_key = "token:";
    static constexpr int token_expire = 60 * 60; // 1 hour, seconds
}