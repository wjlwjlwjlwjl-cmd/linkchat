#include "transmite_manager.h"
#include "random_id.h"
#include "service_constants.h"
#include "jwt_util.h"
#include "base.pb.h"
#include <google/protobuf/util/json_util.h>

namespace {

// 仅在发送到消息队列时，将 C++ 消息结构转换为 protobuf JSON
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

TransmiteManager::TransmiteManager(TransmiteDao::Ptr dao, 
    const std::string& exchange, const std::string& binding_key, MqClient::Ptr mq_client){
    dao_ = dao;
    exchange_ = exchange;
    binding_key_ = binding_key;
    mq_client_ = mq_client;
}

TransmiteManager::~TransmiteManager() = default;

GetTransmitTargetDTO TransmiteManager::GetTransmitTarget(const NewMessageDTO& req){
    GetTransmitTargetDTO rsp;

    // 1. 从req 中提取出发送者 id，从用户服务 rpc 获取元信息
    std::string user_id;
    if(!JwtParse(req.token, "user_id", user_id)){
        rsp.errmsg = "Invalid token";
        rsp.success = false;
        return rsp;
    }

    auto sender_info = dao_->GetSenderInfo(user_id); // 发送者元信息
    if(!sender_info.errmsg.empty()){
        rsp.success = false;
        rsp.errmsg = sender_info.errmsg;
        return rsp;
    }
    UserInfo user_info;
    user_info.user_id = sender_info.user_info.user_id;
    user_info.nickname = sender_info.user_info.nickname;
    user_info.avatar = sender_info.user_info.avatar;
    user_info.description = sender_info.user_info.description;
    user_info.email = sender_info.user_info.email;

    // 2. 给消息分配id，向消息存储子服务发送异步信息（RabbitMQ，由对端 OnMessage 处理，只发送消息核心字段 MessageInfo 即可）
    MessageInfo message_info;
    message_info.message_id = "msg_" + random_util::GetInstanceId();
    message_info.chat_session_id = req.chat_session_id;
    message_info.sender = user_info;
    message_info.message = req.message;

    // 队列的声明操作放到 main 函数中，这里直接发送
    // 对齐交换机、队列等
    // --amqp_broker=amqp://admin:123456@rabbitmq-service:5672
    // --amqp_exchange=msg_exchange
    // --amqp_exchange_type=direct
    // --amqp_queue=msg_queue
    // --amqp_binding_key=msg_key
    ::linkchat::MessageInfo proto_message;
    ToProtoMessageInfo(message_info, &proto_message);
    std::string content;
    ::google::protobuf::util::MessageToJsonString(proto_message, &content);
    if(!mq_client_->Publish(exchange_, binding_key_, content.c_str())){
        // 异步消息持久化失败，直接这次全部失败 
        rsp.success = false;
        rsp.errmsg = "Publish message to mq failed";
        return rsp;
    }

    //todo
    // 3. 根据 chat_session_id 这个会话所有的用户id
    // 这部分需要到下个子服务完成数据库结构搭建，这里先跳过 
    std::vector<std::string> target_ids;

    // 4. 将消息内容、会话id、发送者元信息等打包成 rsp 返回
    rsp.success = true;
    rsp.message = message_info;
    rsp.target_id_list = target_ids;

    return rsp;
}
