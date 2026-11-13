#include "transmite_server.h"
#include <brpc/closure_guard.h>

TransmiteServer::TransmiteServer(TransmiteManager::Ptr transmite_manager)
    : transmite_manager_(transmite_manager) {
}

TransmiteServer::~TransmiteServer() = default;

void TransmiteServer::GetTransmitTarget(::google::protobuf::RpcController* controller,
                    const ::linkchat::NewMessageReq* request,
                    ::linkchat::GetTransmitTargetRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);
    *response = transmite_manager_->GetTransmitTarget(*request);
}