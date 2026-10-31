// 负责oss文件存储，上限 20 MB，上传提供文件名，file_id 需要文件后缀
// 获取通过 file_id 获取，存储调用方自行完成 file_id <-> file_name 的对应持久化


#pragma once
#include <brpc/server.h>
#include <gflags/gflags.h>
#include <butil/logging.h>

#include "file.pb.h"
#include "service/file_manager.h"

using namespace linkchat;
class FileServer: public FileService{
public:
    FileServer(FileManager::Ptr service);

    void GetSingleFile(::google::protobuf::RpcController* controller,
                       const ::linkchat::GetSingleFileReq* request,
                       ::linkchat::GetSingleFileRsp* response,
                       ::google::protobuf::Closure* done) override;

    void PutSingleFile(::google::protobuf::RpcController* controller,
                       const ::linkchat::PutSingleFileReq* request,
                       ::linkchat::PutSingleFileRsp* response,
                       ::google::protobuf::Closure* done) override;
private:
    FileManager::Ptr service_;
};