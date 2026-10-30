#pragma once

#include <unordered_map>
#include <vector>

#include "oss/oss_client.h"
#include "file.pb.h"

/*
要提供的方法：
1. 获取一个文件
2. 获取多个文件
3. 上传单个文件
4. 上传多个文件
*/

class OssDao{
public:
    using Ptr = std::shared_ptr<OssDao>;

    OssDao(const std::string& endpoint, const std::string& region, const std::string& bucket_name,
    const std::string& access_id, const std::string& access_key, int64_t url_expiration = 900);

    /// @brief 下载单个文件
    /// @param  file_id
    /// @return presigned url
    virtual std::string GetSingleFile(const std::string&);

    /// @brief 上传单个文件
    /// @param file_content 文件内容
    /// @param file_name 文件名
    /// @return file_id
    virtual std::string PutSingleFile(const std::string& file_content, const std::string& file_name);
private:
    OssClient::Ptr client_;
};