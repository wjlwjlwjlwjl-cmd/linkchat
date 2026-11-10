#include <string>

namespace redis_constants{
    static constexpr const char* email_verify_code_key = "email_verify_code:";
    static constexpr int email_verify_code_expire = 5 * 60 * 1000; // 5 minutes

    static constexpr const char* token_key = "token:";
    static constexpr int token_expire = 60 * 60 * 1000; // 1 hour
}