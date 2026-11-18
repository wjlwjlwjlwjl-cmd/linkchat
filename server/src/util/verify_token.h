#pragma once

#include <chrono>
#include <memory>
#include <string>

#include <sw/redis++/redis.h>

#include "jwt_util.h"
#include "redis_constants.h"

/// @brief 校验登录令牌
/// @param token 登录接口返回的 JWT 令牌
/// @param redis_template Redis 连接，用于检查 token 是否有效并续租
/// @return 是否校验通过
inline bool VerifyToken(const std::string& token,
                        const std::shared_ptr<sw::redis::Redis>& redis_template) {
    // 这里检查 jwt token 能否成功解析，能则认为jwt合法，如果不合法，直接 401 Unauthorized
    if(token.empty()) return false;
    std::string user_id;
    if(!JwtParse(token, "user_id", user_id)){
        return false;
    }
    if(user_id == "") return false;

    // jwt 合法，检查是否过期，存在则“续租”
    std::string token_key = redis_constants::token_key + user_id;
    if(!redis_template->exists(token_key)){
        return false;
    }
    redis_template->expire(token_key, std::chrono::seconds(redis_constants::token_expire));
    return true;
}
