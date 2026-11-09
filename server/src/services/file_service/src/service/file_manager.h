#pragma once
#include "file.pb.h"

#include "dao/OssDao.h"

class FileManager{
public:
    using Ptr = std::shared_ptr<FileManager>;
    FileManager(OssDao::Ptr dao);

    /// @brief 根据 file_id 到获取预签名连接
    /// @param req 
    /// @return 包含 预签名链接
    ::linkchat::GetSingleFileRsp GetSingleFile(const ::linkchat::GetSingleFileReq* req);

    /// @brief 上传单个文件
    /// @param req 包含文件内容
    /// @return 包含 file_id
    linkchat::PutSingleFileRsp PutSingleFile(const ::linkchat::PutSingleFileReq* req);
private:
    OssDao::Ptr dao_;
};