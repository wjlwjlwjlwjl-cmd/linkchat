#include "file_controller.h"

FileController::FileController(FileManager::Ptr service){
    service_ = service;
}

void FileController::GetSingleFile(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetSingleFileReq* request,
                    ::linkchat::GetSingleFileRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard closure_guard(done);
    auto ret = service_->GetSingleFile(request->file_id());

    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
    response->mutable_file_data()->set_file_id(ret.file_data.file_id);
    response->mutable_file_data()->set_presigned_url(ret.file_data.presigned_url);
}

void FileController::GetMultiFile(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetMultiFileReq* request,
                    ::linkchat::GetMultiFileRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard closure_guard(done);
    std::string errmsg;
    for(auto& file_id: request->file_ids()){
        auto ret = service_->GetSingleFile(file_id);
        if(ret.success){
            ::linkchat::FileDownloadData file_data;
            file_data.set_file_id(ret.file_data.file_id);
            file_data.set_presigned_url(ret.file_data.presigned_url);
            response->mutable_file_data()->insert({file_id, file_data});
        }
        else{
            errmsg = ret.errmsg; //遇到第一个获取失败的停下
            break;
        }
    }
    if(!errmsg.empty()){
        response->set_success(false);
        response->set_errmsg(errmsg);
    }
    else{
        response->set_success(true);
    }
}

void FileController::PutSingleFile(::google::protobuf::RpcController* controller,
                    const ::linkchat::PutSingleFileReq* request,
                    ::linkchat::PutSingleFileRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard closure_guard(done);
    std::string file_name = request->file_data().file_name();
    std::string file_content = request->file_data().file_content();
    auto ret = service_->PutSingleFile(file_name, file_content);

    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
    response->mutable_file_info()->set_file_id(ret.file_info.file_id.value_or(""));
    response->mutable_file_info()->set_file_name(request->file_data().file_name());
}

void FileController::PutMultiFile(::google::protobuf::RpcController* controller,
                    const ::linkchat::PutMultiFileReq* request,
                    ::linkchat::PutMultiFileRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard closure_guard(done);
    std::string errmsg;
    for(auto& file_data: request->file_data()){
        std::string file_name = file_data.file_name();
        std::string file_content = file_data.file_content();
        auto ret = service_->PutSingleFile(file_name, file_content);
        if(!ret.success){
            errmsg = ret.errmsg; //遇到第一个上传失败的停下
            break;
        }
        else{
            ::linkchat::FileMessageInfo file_info;
            file_info.set_file_id(ret.file_info.file_id.value_or(""));
            file_info.set_file_name(file_name);
            response->mutable_file_info()->insert({file_name, file_info});
        }
    }
    if(!errmsg.empty()){
        response->set_success(false);
        response->set_errmsg(errmsg);
    }
    else{
        response->set_success(true);
    }
}
