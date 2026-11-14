#include "file_manager.h"

FileManager::FileManager(OssDao::Ptr dao){
    dao_ = dao;
}

/// @brief 根据 file_id 到获取预签名连接
/// @param req 
/// @return 包含 预签名链接
GetSingleFileDTO FileManager::GetSingleFile(const std::string& file_id){
    if(file_id == ""){
        GetSingleFileDTO resp;
        resp.success = false;
        resp.errmsg = "file_id is empty";
        return resp;
    }

    GetSingleFileDTO resp;
    std::string presigned_url = dao_->GetSingleFile(file_id);
    if(presigned_url.empty()){
        resp.success = false;
        resp.errmsg = "Fail to get presigned_url";
        return resp;
    }

    resp.success = true;
    resp.file_data.file_id = file_id;
    resp.file_data.presigned_url = presigned_url;
    return resp;
}

/// @brief 上传单个文件
/// @param req 包含文件内容
/// @return 包含 file_id
PutSingleFileDTO FileManager::PutSingleFile(const std::string& file_name, const std::string& file_content){
    PutSingleFileDTO rsp;
    if(file_name == ""){
        rsp.success = false;
        rsp.errmsg = "file_name is empty";
        return rsp;
    }

    std::string file_id = dao_->PutSingleFile(file_content, file_name);

    if(file_id.empty()){
        rsp.success = false;
        rsp.errmsg = "Fail to upload file to Oss";
        return rsp;
    }    
    rsp.success = true;
    rsp.file_info.file_id = file_id;
    return rsp;
}
