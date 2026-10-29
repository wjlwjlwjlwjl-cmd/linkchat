#pragma once

#include <alibabacloud/oss/OssClient.h>
#include <time.h>
#include <sstream>
#include <thread>

class OssClient {
public:
    OssClient(const std::string& endpoint, const std::string& region, const std::string& bucket_name,
        const std::string& access_id, const std::string& access_key, int64_t url_expiration = 900);

    /// @brief 初始化 OSS 客户端
    void Init();

    /// @brief 获取预签名上传 url
    /// @param object_name （路径）文件名
    /// @return 预签名 url
    std::string PresignedDownload(const std::string& object_name);

    /// @brief 上传文件（提供二进制数据即可，大小限制 20MB）
    /// @param object_name 文件名
    /// @return 是否上传成功
    bool Upload(const std::string& file_content, const std::string& object_name);
private:
    std::string endpoint_;
    std::string region_;
    std::string bucket_name_; //按理是可以创建的，但是这里采取不提供代码创建
    std::string access_id_;
    std::string access_key_;
    int64_t url_expiration_;

    std::shared_ptr<AlibabaCloud::OSS::OssClient> client_;
};

inline OssClient::OssClient(const std::string& endpoint, const std::string& region, const std::string& bucket_name,
    const std::string& access_id, const std::string& access_key, int64_t url_expiration)
    : endpoint_(endpoint)
    , region_(region)
    , bucket_name_(bucket_name)
    , access_id_(access_id)
    , access_key_(access_key)
    , url_expiration_(url_expiration)
{}

inline void OssClient::Init() {
    AlibabaCloud::OSS::InitializeSdk(); /*初始化网络资源*/

    AlibabaCloud::OSS::ClientConfiguration conf;
    conf.signatureVersion = AlibabaCloud::OSS::SignatureVersionType::V4;

    auto credential_provider = std::make_shared<AlibabaCloud::OSS::SimpleCredentialsProvider>(access_id_, access_key_);
    client_ = std::make_shared<AlibabaCloud::OSS::OssClient>(endpoint_, credential_provider, conf);
    client_->SetRegion(region_);
}

inline std::string OssClient::PresignedDownload(const std::string& object_name) {
    if (object_name.empty()) return "";

    std::time_t expiration = std::time(nullptr) + url_expiration_;
    auto outcome = client_->GeneratePresignedUrl(bucket_name_, object_name, expiration, AlibabaCloud::OSS::Http::Get);
    if (outcome.isSuccess()) {
        return outcome.result();
    }
    else {
        return ""; //获取下载链接失败
    }
}

inline bool OssClient::Upload(const std::string& file_content, const std::string& object_name) {
    if (object_name.empty()) return false;

    const int64_t kMaxUploadSize = 20 * 1024 * 1024;
    if (kMaxUploadSize < static_cast<int64_t>(file_content.size())) {
        return false;
    }

    auto content = std::make_shared<std::stringstream>();
    content->write(file_content.data(), file_content.size());

    auto outcome = client_->PutObject(bucket_name_, object_name, content);
    if (outcome.isSuccess()) {
        return true;
    }
    return false;
}
