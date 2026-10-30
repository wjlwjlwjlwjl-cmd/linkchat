#include "file_server.h"

FileServer::FileServer(FileManager::Ptr service){
    service_ = service;
}

void FileServer::GetSingleFile(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetSingleFileReq* request,
                    ::linkchat::GetSingleFileRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard closure_guard(done);
    auto ret = service_->GetSingleFile(request);

    response->set_success(ret.success());
    response->set_errmsg(ret.errmsg());
    response->set_request_id(ret.request_id());
    response->mutable_file_data()->set_file_id(ret.file_data().file_id());
    response->mutable_file_data()->set_presigned_url(ret.file_data().presigned_url());
}

void FileServer::PutSingleFile(::google::protobuf::RpcController* controller,
                    const ::linkchat::PutSingleFileReq* request,
                    ::linkchat::PutSingleFileRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard closure_guard(done);
    auto ret = service_->PutSingleFile(request);

    response->set_success(ret.success());
    response->set_errmsg(ret.errmsg());
    response->set_request_id(ret.request_id());
    response->mutable_file_info()->set_file_id(ret.file_info().file_id());
    response->mutable_file_info()->set_file_name(request->file_data().file_name());
    response->mutable_file_info()->set_file_size(request->file_data().file_size());
}
