#pragma once
#include "transmite.pb.h"
#include "service/transmite_manager.h"

class TransmiteController : public ::linkchat::MsgTransmitService{
public:
    TransmiteController(TransmiteManager::Ptr);

    virtual void GetTransmitTarget(::google::protobuf::RpcController* controller,
                       const ::linkchat::NewMessageReq* request,
                       ::linkchat::GetTransmitTargetRsp* response,
                       ::google::protobuf::Closure* done);

    ~TransmiteController();
private:
    TransmiteManager::Ptr transmite_manager_;
};