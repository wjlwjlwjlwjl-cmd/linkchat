#pragma once

#include <alibabacloud/oss/OssClient.h>
#include <time.h>
#include <sstream>
#include <thread>

class OSS{
public:
    OSS(const std::string& endpoint, const std::string& region, const std::string& bucket_name, 
        const std::string& access_id, const std::string& access_key, int64_t url_expiration = 900);

    /// @brief 初始化 OSS 客户端
    void Init();

    /// @brief 获取预签名上传 url
    /// @param object_name （路径）文件名 
    /// @return 预签名 url
    std::string PresignedDownload(const std::string& object_name);

    /// @brief 上传文件（提供二进制数据即可）
    /// @param object_name 文件名
    /// @return 是否上传成功
    bool Upload(const std::string& file_content, const std::string& object_name);
private:
    std::string _endpoint;
    std::string _region;
    std::string _bucket_name; //按理是可以创建的，但是这里采取不提供代码创建
    std::string _access_id;
    std::string _access_key;
    int64_t _url_expiration;

    std::shared_ptr<AlibabaCloud::OSS::OssClient> _client;
};

OSS::OSS(const std::string& endpoint, const std::string& region, const std::string& bucket_name, 
    const std::string& access_id, const std::string& access_key, int64_t url_expiration)
    : _endpoint(endpoint)
    , _region(region)
    , _bucket_name(bucket_name)
    , _access_id(access_id)
    , _access_key(access_key)
    , _url_expiration(url_expiration)
{}

void OSS::Init(){
    AlibabaCloud::OSS::InitializeSdk(); /*初始化网络资源*/

    AlibabaCloud::OSS::ClientConfiguration conf;
    conf.signatureVersion = AlibabaCloud::OSS::SignatureVersionType::V4;

    auto credential_provider = std::make_shared<AlibabaCloud::OSS::SimpleCredentialsProvider>(_access_id, _access_key);
    _client = std::make_shared<AlibabaCloud::OSS::OssClient>(_endpoint, credential_provider, conf);
    _client->SetRegion(_region);
}

std::string OSS::PresignedDownload(const std::string& object_name){
    if(object_name.empty()) return "";

    std::time_t time = std::time(nullptr) + _url_expiration;
    auto outcome = _client->GeneratePresignedUrl(_bucket_name, object_name, time, AlibabaCloud::OSS::Http::Get);
    if(outcome.isSuccess()){
        return outcome.result();
    }
    else{
        return ""; //获取下载链接失败
    }
}

bool OSS::Upload(const std::string& file_content, const std::string& object_name){
    if(object_name.empty()) return "";

    auto content = std::make_shared<std::stringstream>();
    content->write(file_content.data(), file_content.size());

    auto outcome = _client->PutObject(_bucket_name, object_name, content);
    if(outcome.isSuccess()){
        return true;
    }
    return false;
}