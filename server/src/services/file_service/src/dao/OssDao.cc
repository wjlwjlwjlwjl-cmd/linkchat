#include "OssDao.h"
#include "file_util.h"
#include "random_id.h"
#include "string_util.h"

OssDao::OssDao(const std::string& endpoint, const std::string& region, const std::string& bucket_name,
    const std::string& access_id, const std::string& access_key, int64_t url_expiration)
    : client_(std::make_shared<OssClient>(endpoint, region, bucket_name, access_id, access_key, url_expiration))
{
    client_->Init();
}    

std::string OssDao::GetSingleFile(const std::string& file_id){
    std::string presigned_url = client_->PresignedDownload(file_id);
    return presigned_url;
}

std::string OssDao::PutSingleFile(const std::string& file_content, const std::string& file_name){
    std::string file_id = "file_" + random_util::GetInstanceId() + GetFileExtension(file_name);

    if(!client_->Upload(file_content, file_id)){
        return ""; //上传失败
    }

    return file_id;
}