#include "transmite_server.h"
#include <brpc/closure_guard.h>

namespace {

MessageContent FromProtoMessageContent(const ::linkchat::MessageContent& src) {
    MessageContent dst;
    dst.message_type = src.message_type() == ::linkchat::MessageType::STRING
        ? MessageType::STRING : MessageType::FILE;

    if (src.has_string_message()) {
        StringMessageInfo string_message;
        string_message.content = src.string_message().content();
        dst.string_message = std::move(string_message);
    }
    if (src.has_file_message()) {
        FileMessageInfo file_message;
        const auto& file = src.file_message();
        if (file.has_file_id()) {
            file_message.file_id = file.file_id();
        }
        if (file.has_file_size()) {
            file_message.file_size = file.file_size();
        }
        if (file.has_file_name()) {
            file_message.file_name = file.file_name();
        }
        if (file.has_file_content()) {
            file_message.file_content = file.file_content();
        }
        dst.file_message = std::move(file_message);
    }
    return dst;
}

void ToProtoMessageInfo(const MessageInfo& src, ::linkchat::MessageInfo* dst) {
    dst->set_message_id(src.message_id);
    dst->set_chat_session_id(src.chat_session_id);

    auto* sender = dst->mutable_sender();
    sender->set_user_id(src.sender.user_id);
    sender->set_nickname(src.sender.nickname);
    sender->set_description(src.sender.description);
    sender->set_email(src.sender.email);
    sender->set_avatar(src.sender.avatar);

    auto* message = dst->mutable_message();
    message->set_message_type(src.message.message_type == MessageType::STRING
        ? ::linkchat::MessageType::STRING : ::linkchat::MessageType::FILE);
    if (src.message.string_message) {
        message->mutable_string_message()->set_content(src.message.string_message->content);
    }
    if (src.message.file_message) {
        auto* file_message = message->mutable_file_message();
        if (src.message.file_message->file_id) {
            file_message->set_file_id(*src.message.file_message->file_id);
        }
        if (src.message.file_message->file_size) {
            file_message->set_file_size(*src.message.file_message->file_size);
        }
        if (src.message.file_message->file_name) {
            file_message->set_file_name(*src.message.file_message->file_name);
        }
        if (src.message.file_message->file_content) {
            file_message->set_file_content(*src.message.file_message->file_content);
        }
    }
}

} // namespace

TransmiteServer::TransmiteServer(TransmiteManager::Ptr transmite_manager)
    : transmite_manager_(transmite_manager) {
}

TransmiteServer::~TransmiteServer() = default;

void TransmiteServer::GetTransmitTarget(::google::protobuf::RpcController* controller,
                    const ::linkchat::NewMessageReq* request,
                    ::linkchat::GetTransmitTargetRsp* response,
                    ::google::protobuf::Closure* done){
    ::brpc::ClosureGuard guard(done);

    NewMessageDTO req;
    req.token = request->token();
    req.chat_session_id = request->chat_session_id();
    req.message = FromProtoMessageContent(request->message());

    auto ret = transmite_manager_->GetTransmitTarget(req);

    response->set_success(ret.success);
    response->set_errmsg(ret.errmsg);
    ToProtoMessageInfo(ret.message, response->mutable_message());
    response->mutable_target_id_list()->Add(ret.target_id_list.begin(), ret.target_id_list.end());
}
