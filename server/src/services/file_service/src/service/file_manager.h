#pragma once
#include "domain/file_dto.h"

#include "dao/OssDao.h"

class FileManager{
public:
    using Ptr = std::shared_ptr<FileManager>;
    FileManager(OssDao::Ptr dao);

    /// @brief 根据 file_id 到获取预签名连接
    /// @param req 
    /// @return 包含 预签名链接
    GetSingleFileDTO GetSingleFile(const std::string& file_id);

    /// @brief 上传单个文件
    /// @param req 包含文件内容
    /// @return 包含 file_id
    PutSingleFileDTO PutSingleFile(const std::string& file_name, const std::string& file_content);
private:
    OssDao::Ptr dao_;
};