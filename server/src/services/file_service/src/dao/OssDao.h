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

class FileService{
public:
    FileService();
    FileService(const std::string& endpoint, const std::string& region, const std::string& bucket_name,
        const std::string& access_id, const std::string& access_key, int64_t url_expiration = 900);    

    linkchat::FileDownloadData GetSingleFile(const std::string&);
    std::unordered_map<std::string, linkchat::FileDownloadData> GetMultipleFile(const std::vector<std::string>&);
    linkchat::FileMessageInfo PutSingleFile(const linkchat::FileUploadData&);
    std::vector<linkchat::FileMessageInfo> PutMultipleFile(const std::vector<linkchat::FileUploadData>&);
};