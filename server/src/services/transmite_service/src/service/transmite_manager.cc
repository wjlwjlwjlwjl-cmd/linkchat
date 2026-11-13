#include "transmite_manager.h"
#include "random_id.h"
#include "service_constants.h"

TransmiteManager::TransmiteManager(TransmiteDao::Ptr dao, 
    const std::string& exchange, const std::string& binding_key, MqClient::Ptr mq_client){
    dao_ = dao;
    exchange_ = exchange;
    binding_key_ = binding_key;
    mq_client_ = mq_client;
}

::linkchat::GetTransmitTargetRsp TransmiteManager::GetTransmitTarget(const ::linkchat::NewMessageReq& req){
    ::linkchat::GetTransmitTargetRsp rsp;

    // 1. 从req 中提取出发送者 id，从用户服务 rpc 获取元信息
    std::string token = req.token();
    std::string user_id;
    if(!JwtParse(token, "user_id", user_id)){
        rsp.set_errmsg("Invalid token");
        rsp.set_success(false);
        return rsp;
    }

    auto sender_info = dao_->GetSenderInfo(user_id); // 发送者元信息
    if(!sender_info.errmsg.empty()){
        rsp.set_success(false);
        rsp.set_errmsg(sender_info.errmsg);
        return rsp;
    }
    ::linkchat::UserInfo user_info;
    user_info.set_user_id(user_info.user_id());
    user_info.set_nickname(sender_info.nickname);
    user_info.set_avatar(sender_info.avatar);
    user_info.set_description(sender_info.description);
    user_info.set_email(user_info.email());

    // 2. 给消息分配id，向消息存储子服务发送异步信息（RabbitMQ，由对端 OnMessage 处理，只发送消息核心字段 MessageInfo 即可）
    ::linkchat::MessageInfo message_info;

    std::string message_id = "msg_" + random_util::GetInstanceId();
    message_info.set_message_id(message_id);
    message_info.set_chat_session_id(req.chat_session_id());
    message_info.mutable_sender()->CopyFrom(user_info);
    message_info.mutable_message()->CopyFrom(req.message());

    // 队列的声明操作放到 main 函数中，这里直接发送
    // 对齐交换机、队列等
    // --amqp_broker=amqp://admin:123456@rabbitmq-service:5672
    // --amqp_exchange=msg_exchange
    // --amqp_exchange_type=direct
    // --amqp_queue=msg_queue
    // --amqp_binding_key=msg_key
    std::string content;
    ::google::protobuf::util::MessageToJsonString(message_info, &content);
    if(!mq_client_->Publish(exchange_, binding_key_, content.c_str())){
        // 异步消息持久化失败，直接这次全部失败 
        rsp.set_success(false);
        rsp.set_errmsg("Publish message to mq failed");
        return rsp;
    }

    //todo
    // 3. 根据 chat_session_id 这个会话所有的用户id
    // 这部分需要到下个子服务完成数据库结构搭建，这里先跳过 
    std::vector<std::string> target_ids;

    // 4. 将消息内容、会话id、发送者元信息等打包成 rsp 返回
    rsp.set_success(true);
    rsp.mutable_message()->CopyFrom(message_info);
    rsp.mutable_target_id_list()->Add(target_ids.begin(), target_ids.end());

    return rsp;
}