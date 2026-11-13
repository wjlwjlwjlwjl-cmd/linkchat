#pragma once
#include "transmite.pb.h"

class TransmiteServer : public ::linkchat::MsgTransmitService{
public:
    TransmiteServer();

    virtual void GetTransmitTarget(::google::protobuf::RpcController* controller,
                       const ::linkchat::NewMessageReq* request,
                       ::linkchat::GetTransmitTargetRsp* response,
                       ::google::protobuf::Closure* done);

    ~TransmiteServer();
};