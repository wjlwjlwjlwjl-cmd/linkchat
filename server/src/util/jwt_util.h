#pragma once

#include <jwt-cpp/jwt.h>

static constexpr const char* secret = "Hello World!";

/// @brief 创建 JWT 令牌
/// @param key 令牌中的键
/// @param value 令牌中的值
/// @return JWT 令牌
inline std::string JwtCreate(const std::string& key,
                       const std::string& value) {
    return jwt::create()
        .set_type("JWS")
        .set_payload_claim(key, jwt::claim(value))
        .sign(jwt::algorithm::hs256{secret});
}

/// @brief 解析 JWT 令牌
/// @param secret 密钥
/// @param token JWT 令牌
/// @param key 令牌中的键
/// @param value 令牌中的值，输出型参数
/// @return 是否解析成功
inline bool JwtParse(const std::string& token,
               const std::string& key,
               std::string& value) {
    try {
        auto decoded = jwt::decode(token);
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret})
            .verify(decoded);
        value = decoded.get_payload_claim(key).as_string();
        return true;
    } catch (...) {
        return false;
    }
}