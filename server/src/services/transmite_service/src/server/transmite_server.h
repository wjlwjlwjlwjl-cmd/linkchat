#pragma once
#include "transmite.pb.h"
#include "service/transmite_manager.h"

class TransmiteServer : public ::linkchat::MsgTransmitService{
public:
    TransmiteServer(TransmiteManager::Ptr);

    virtual void GetTransmitTarget(::google::protobuf::RpcController* controller,
                       const ::linkchat::NewMessageReq* request,
                       ::linkchat::GetTransmitTargetRsp* response,
                       ::google::protobuf::Closure* done);

    ~TransmiteServer();
private:
    TransmiteManager::Ptr transmite_manager_;
};