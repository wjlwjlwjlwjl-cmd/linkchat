#include "dao/OssDao.h"
#include "file.pb.h"
#include "file_manager.h"

FileManager::FileManager(OssDao::Ptr dao){
    dao_ = dao;
}

/// @brief 根据 file_id 到获取预签名连接
/// @param req 
/// @return 包含 预签名链接
::linkchat::GetSingleFileRsp FileManager::GetSingleFile(const std::string& file_id){
    if(file_id == ""){
        ::linkchat::GetSingleFileRsp resp;
        resp.set_success(false);
        resp.set_errmsg("file_id is empty");
        return resp;
    }

    ::linkchat::GetSingleFileRsp resp;
    std::string presigned_url = dao_->GetSingleFile(file_id);
    if(presigned_url.empty()){
        resp.set_success(false);
        resp.set_errmsg("Fail to get presigned_url");
        return resp;
    }

    resp.set_success(true);
    resp.mutable_file_data()->set_file_id(file_id);
    resp.mutable_file_data()->set_presigned_url(presigned_url);
    return resp;
}

/// @brief 上传单个文件
/// @param req 包含文件内容
/// @return 包含 file_id
::linkchat::PutSingleFileRsp FileManager::PutSingleFile(const std::string& file_name, const std::string& file_content){
    ::linkchat::PutSingleFileRsp rsp;
    if(file_name == ""){
        rsp.set_success(false);
        rsp.set_errmsg("file_name is empty");
        return rsp;
    }

    std::string file_id = dao_->PutSingleFile(file_content, file_name);

    if(file_id.empty()){
        rsp.set_success(false);
        rsp.set_errmsg("Fail to upload file to Oss");
        return rsp;
    }    
    rsp.set_success(true);
    rsp.mutable_file_info()->set_file_id(file_id);
    return rsp;
}