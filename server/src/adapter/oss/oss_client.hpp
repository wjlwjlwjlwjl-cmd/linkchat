#pragma once

#include <alibabacloud/oss/OssClient.h>

class OSS{
public:
    OSS(const std::string& endpoint, const std::string& region, const std::string bucket_name, 
        const std::string& access_id, const std::string& access_key, int64_t url_expiration = 900);

    /// @brief 初始化 OSS 客户端
    void Init();

    /// @brief 获取预签名上传 url
    /// @param object_name （路径）文件名 
    /// @return 预签名 url
    std::string PresignedDownload(const std::string& object_name);
private:
    std::string _endpoint;
    std::string _region;
    std::string _bucket_name; //按理是可以创建的，但是这里采取不提供代码创建
    std::string _access_id;
    std::string _access_key;
    int64_t _url_expiration;

    std::shared_ptr<AlibabaCloud::OSS::OssClient> _client;
};

OSS::OSS(const std::string& endpoint, const std::string& region, const std::string bucket_name, 
    const std::string& access_id, const std::string& access_key, int64_t url_expiration = 900)
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
    if(object_name.empty()) return;
    auto outcome = _client->GeneratePresignedUrl(_bucket_name, object_name, _url_expiration, AlibabaCloud::OSS::Http::Get);
    if(outcome.isSuccess()){
        return outcome.result();
    }
    else{
        return ""; //获取下载链接失败
    }
}