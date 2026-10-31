#include "message_manager.h"
#include "etcd/discover/service_discovery.h"
#include "rpc/channels.h"
#include "service_name.h"

MessageManager::MessageManager(
    const std::string& etcd_server_ip, const std::string& protocol, int etcd_port, int connect_timeout_ms,
    int timeout_ms, int max_retry,
    const char* user, const char* passwd, const char* db_name, const char* host,
    unsigned int port, int connection_pool_size,
    const char* elastic_url, const char* index_name,
    const char* doc_type, const char* id){

    dao_ = std::make_shared<MessageDao>(user, passwd, db_name, host, port, connection_pool_size,
        elastic_url, index_name, doc_type, id);

    SvcDiscovery::ModCallback cb_online = std::function<void(const std::string&, const std::string&)>();
    SvcDiscovery::ModCallback cb_offline = std::function<void(const std::string&, const std::string&)>();
    svc_discovery_ = std::make_shared<SvcDiscovery>(etcd_server_ip, std::move(cb_online), std::move(cb_offline));

    channel_options_.connect_timeout_ms = connect_timeout_ms;
    channel_options_.max_retry = max_retry;
    channel_options_.timeout_ms = timeout_ms;
    channel_options_.protocol = protocol;
}

bool MessageManager::PersistMessage(::linkchat::MessageInfo& info, std::string& file_id, 
    std::string& errmsg){
    // 取出消息的 metadata，并对所有消息的消息原信息进行存储
    std::string message_id = info.message_id();
    std::string chat_session_id = info.chat_session_id();
    std::string user_id = info.sender().user_id();
    ::linkchat::MessageType message_type = info.message().message_type();
    dao_->MsgMetadataStore(message_id, chat_session_id, user_id, message_type);

    if(message_type == ::linkchat::MessageType::STRING){
        if(!info.message().has_string_message()){
            LOG_WARN("string message {} has wrong content", info.message_id());
            return false;
        }
        std::string content = info.message().string_message().content();
        return dao_->MsgElasticStore(content, chat_session_id, message_id, user_id);
    }
    else if(message_type == ::linkchat::MessageType::FILE){
        // 对于文件、图片类型的消息，上传阿里云对象存储
        ChannelPtr channel = svc_discovery_->GetSvcChannel(service_name::file_service);
        std::string ip = svc_discovery_->GetSvcIp(service_name::file_service);

        if (channel->Init(ip.c_str(), &channel_options_) != 0){
            LOG_ERROR("Channel Init fail: {}", message_id);
            return false;
        }

        ::linkchat::FileService_Stub stub(channel.get());
        ::linkchat::PutSingleFileReq req;

        //这里暂时没有处理 request_id
        std::string file_name = info.message().file_message().file_name();
        std::string file_contents = info.message().file_message().file_contents();
        req.mutable_file_data()->set_file_name(file_name);
        req.mutable_file_data()->set_file_content(file_contents);
        
        auto* controller = new ::brpc::Controller();
        auto* response = new ::linkchat::PutSingleFileRsp();
        stub.PutSingleFile(controller, &req, response, nullptr); //这里采取同步调用，等待结果
        if(response->success() == false){
            file_id = "";
            errmsg = response->errmsg();
            return false;
        }
        else{
            file_id = response->file_info().file_id();
            return true;
        }
    }
    else{
        LOG_ERROR("Unknown MessageType {}", message_id);
        return false;
    }
    return false;
}
