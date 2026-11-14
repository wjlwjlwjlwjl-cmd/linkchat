#include "message_controller.h"
#include "base.pb.h"
#include <brpc/closure_guard.h>

namespace {

void ToProtoMessage(const MessageInfo& src, ::linkchat::MessageInfo* dst) {
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

MessageInfo FromProtoMessage(const ::linkchat::MessageInfo& src) {
    MessageInfo dst;
    dst.message_id = src.message_id();
    dst.chat_session_id = src.chat_session_id();
    dst.sender.user_id = src.sender().user_id();
    dst.sender.nickname = src.sender().nickname();
    dst.sender.description = src.sender().description();
    dst.sender.email = src.sender().email();
    dst.sender.avatar = src.sender().avatar();

    dst.message.message_type = src.message().message_type() == ::linkchat::MessageType::STRING
        ? MessageType::STRING : MessageType::FILE;

    if (src.message().has_string_message()) {
        StringMessageInfo string_message;
        string_message.content = src.message().string_message().content();
        dst.message.string_message = std::move(string_message);
    }
    if (src.message().has_file_message()) {
        FileMessageInfo file_message;
        const auto& file = src.message().file_message();
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
        dst.message.file_message = std::move(file_message);
    }
    return dst;
}

void AddMessageList(const std::vector<MessageInfo>& msgs,
    ::google::protobuf::RepeatedPtrField<::linkchat::MessageInfo>* msg_list) {
    for (const auto& msg : msgs) {
        ToProtoMessage(msg, msg_list->Add());
    }
}

} // namespace

MessageController::MessageController(
    const std::string& etcd_server_ip, const std::string& protocol, int connect_timeout_ms,
            int timeout_ms, int max_retry,
            const char* user, const char* passwd, const char* db_name, const char* host,
            unsigned int port, int connection_pool_size,
            const char* elastic_url, const char* index_name, const char* doc_type, const char* id
){
    manager_ = std::make_shared<MessageManager>(etcd_server_ip, protocol, connect_timeout_ms,
        timeout_ms, max_retry, user, passwd, db_name, host, port, connection_pool_size, elastic_url,
        index_name, doc_type);
}

void MessageController::GetHistoryMsg(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetHistoryMsgReq* request,
                    ::linkchat::GetHistoryMsgRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard guard(done);
    auto msgs = manager_->GetHistoryMsg(request->chat_session_id(), request->start_time(),
        request->over_time(), request->token());
    AddMessageList(msgs, response->mutable_msg_list());
    response->set_success(true);
}

void MessageController::GetRecentMsg(::google::protobuf::RpcController* controller,
                    const ::linkchat::GetRecentMsgReq* request,
                    ::linkchat::GetRecentMsgRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard guard(done);
    auto msgs = manager_->GetRecentMsg(request->chat_session_id(), request->msg_count(),
        request->cur_time(), request->token());
    AddMessageList(msgs, response->mutable_msg_list());
    response->set_success(true);
}

void MessageController::MsgSearch(::google::protobuf::RpcController* controller,
                    const ::linkchat::MsgSearchReq* request,
                    ::linkchat::MsgSearchRsp* response,
                    ::google::protobuf::Closure* done){
    brpc::ClosureGuard guard(done);
    if(request->chat_session_id().empty() || request->search_key().empty()){
        response->set_success(false);
        response->set_errmsg("chat_session_id and search_key must not be empty");
        return;
    }
    auto msgs = manager_->MsgSearch(request->chat_session_id(), request->search_key(), request->token());
    AddMessageList(msgs, response->mutable_msg_list());
    response->set_success(true);
}

bool MessageController::OnMessage(const char* message, int size){
    std::string raw = std::string(message, size);
    ::linkchat::MessageInfo message_info;
    auto status =::google::protobuf::util::JsonStringToMessage(raw, &message_info);
    if(!status.ok()){
        LOG_ERROR("Failed to deserialize message: {}", status.ToString());
        return false;
    }
    return manager_->PersistMessage(FromProtoMessage(message_info));
}
