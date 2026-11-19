#pragma once

#include <botan-3/botan/argon2fmt.h>
#include <botan-3/botan/auto_rng.h>

// 密码的加盐存储
inline std::string create_password_hash(const std::string& password) {
    Botan::AutoSeeded_RNG rng;
    // 参数说明：密码, 长度, RNG, p(并行度), M(内存KB), t(迭代次数), 变体(2=Argon2id), 盐长度, 输出长度
    return Botan::argon2_generate_pwhash(
        password.data(), password.size(),
        rng,
        1,        // p=1
        65536,    // M=64MB（可根据机器调整）
        3,        // t=3
        2,        // 2 代表 Argon2id（推荐）
        16,       // 盐长度
        32        // 输出哈希长度
    );
}

// 密码的带盐匹配
inline bool verify_password(const std::string& input_password, const std::string& stored_hash) {
    return Botan::argon2_check_pwhash(
        input_password.data(), input_password.size(),
        stored_hash
    );
}