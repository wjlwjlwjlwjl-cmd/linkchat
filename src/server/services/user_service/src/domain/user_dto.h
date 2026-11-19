#pragma once
#include <string>

/// @brief 用户DTO
struct UserDTO{
    /// @brief 用户ID
    std::string user_id;

    /// @brief 用户昵称
    std::string nickname; 

    /// @brief 用户签名
    std::string description;

    /// @brief 用户密码
    std::string password; 

    /// @brief 用户邮箱
    std::string email; 

    /// @brief 用户头像文件ID
    std::string avatar_id; 

    /// @brief 是否成功
    bool success = false;
};