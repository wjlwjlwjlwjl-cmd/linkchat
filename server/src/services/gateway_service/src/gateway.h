#pragma once
// 现在用户信息，统一存放在 jwt token 中，jwt token 首先用于鉴权，后续用于传递处理用户信息
// Get 方法参数，直接获取即可
// Post 方法参数，如果没有文件操作，body 为 application/json
// 如果有文件操作，body 为 multipart/form-data
// 除白名单接口（/login/mail_verify_code、/register、/login/nickname、/login/email）外，
// 所有接口都必须在请求头携带 Authorization: <登录接口返回的 token>，否则返回 401

#include <brpc/controller.h>
#include <httplib.h>
#include <chrono>
#include <string>
#include <thread>
#include <brpc/channel.h>
#include <functional>
#include <google/protobuf/util/json_util.h>
#include <websocketpp/close.hpp>

#include "user.pb.h"
#include "file.pb.h"
#include "message.pb.h"
#include "transmite.pb.h"
#include "friend.pb.h"

#include "logging/logger.h"
#include "etcd/discover/service_discovery.h"
#include "service_constants.h"
#include "jwt_util.h"
#include "verify_token.h"
#include "redis/redis_factory.h"
#include "redis_constants.h"
#include "json/json_codec.h"
#include "connection.h"
#include "error_msg.h"

using message_t = server_t::message_ptr;

class Gateway{
public:
    //FileService
    #define GET_SINGLE_FILE "/file/get_single"
    #define GET_MULTI_FILE "/file/get_multi"

    // MessageService
    #define GET_HISTORY_MSG "/message/history" // 最近时间段
    #define GET_RECENT_MSG "/message/recent" // 获取最近 N 条
    #define MSG_SEARCH "/message/search" // 搜索消息（ES）

    // UserService
    #define GET_EMAIL_VERIFY_CODE "/login/mail_verify_code"
    #define REGISTER "/register"
    #define NICKNAME_LOGIN "/login/nickname"
    #define EMAIL_LOGIN "/login/email"
    #define GET_USER_INFO "/user/info"
    #define GET_MULTI_USER_INFO "/user/multi_info"
    #define SET_USER_AVATAR "/user/avatar/set"
    #define SET_USER_DESCRIPTION "/user/description/set"
    #define SET_USER_NICKNAME "/user/nickname/set"

    // TransmiteService
    #define NEW_MESSAGE "/transmite/publish"

    // FriendService
    #define GET_FRIEND_LIST "/friend/list"
    #define FRIEND_REMOVE "/friend/remove"
    #define FRIEND_ADD "/friend/add"
    #define FRIEND_ADD_PROCESS "/friend/add_process"
    #define FRIEND_SEARCH "/friend/search"
    #define GET_CHAT_SESSION_LIST "/friend/chat_session/list"
    #define CHAT_SESSION_CREATE "/friend/chat_session/create"
    #define GET_CHAT_SESSION_MEMBER "/friend/chat_session/member"
    #define GET_PENDING_FRIEND_EVENT "/friend/pending_event"

    Gateway(std::shared_ptr<::brpc::ChannelOptions> options, SvcDiscovery::Ptr svc_discovery, 
        redis_factory::RedisConnectionInfo& redis_connection_info, int ws_port){
        options_ = options;
        svc_discovery_ = svc_discovery;
        ws_port_ = ws_port;
        redis_template_ = redis_factory::RedisFactory::Create(redis_connection_info);

        // 设置网关统一身份验证
        http_server_.set_pre_routing_handler([this](const ::httplib::Request& req, ::httplib::Response& res){
            // 处理跨域时浏览器的预检请求
            if (req.method == "OPTIONS") {
                res.set_header("Access-Control-Allow-Origin", "*"); 
                res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
                res.status = 204;
                return ::httplib::Server::HandlerResponse::Handled; // 直接结束
            }

            // 白名单直接放行
            if(white_lists.find(req.path) != white_lists.end()){
                return ::httplib::Server::HandlerResponse::Unhandled;
            }

            // 验证登录态
            std::string token = req.get_header_value("Authorization");
            if(token.empty()){
                res.status = 401;
                return ::httplib::Server::HandlerResponse::Handled;
            }
            if(!VerifyToken(token, redis_template_)){
                res.status = 401;
                return ::httplib::Server::HandlerResponse::Handled;
            }
            return ::httplib::Server::HandlerResponse::Unhandled;
        });

        // 拉取服务信息到本地
        svc_discovery_->Watch();

        ///////////////////////////////
        // FileService
        ///////////////////////////////
        http_server_.Get(GET_SINGLE_FILE, 
            std::bind(&::Gateway::GetSingleFile, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Get(GET_MULTI_FILE, 
            std::bind(&::Gateway::GetMultiFile, this, std::placeholders::_1, std::placeholders::_2));

        ///////////////////////////////
        /// MessageService
        ///////////////////////////////
        http_server_.Get(GET_HISTORY_MSG, 
            std::bind(&::Gateway::GetHistroyMsg, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Get(GET_RECENT_MSG, 
            std::bind(&::Gateway::GetRecentMsg, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Get(MSG_SEARCH, 
            std::bind(&::Gateway::MsgSearch, this, std::placeholders::_1, std::placeholders::_2));

        ///////////////////////////////
        // UserService
        ///////////////////////////////
        http_server_.Get(GET_EMAIL_VERIFY_CODE, 
            std::bind(&Gateway::GetEmailVerifyCode, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Post(REGISTER, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                Register(req, res);
            });
        http_server_.Post(NICKNAME_LOGIN, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                NicknameLogin(req, res);
            });
        http_server_.Post(EMAIL_LOGIN, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                EmailLogin(req, res);
            });
        http_server_.Get(GET_USER_INFO, 
            std::bind(&Gateway::GetUserInfo, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Get(GET_MULTI_USER_INFO, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                GetMultiUserInfo(req, res);
            });
        http_server_.Post(SET_USER_AVATAR, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                SetUserAvatar(req, res);
            });
        http_server_.Post(SET_USER_DESCRIPTION, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                SetUserDescription(req, res);
            });
        http_server_.Post(SET_USER_NICKNAME, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                SetUserNickname(req, res);
            });
        
        ///////////////////////////////
        // MsgTransmitService
        ///////////////////////////////
        http_server_.Post(NEW_MESSAGE, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                NewMessage(req, res);
            });

        ///////////////////////////////
        // FriendService
        ///////////////////////////////
        http_server_.Get(GET_FRIEND_LIST, 
            std::bind(&Gateway::GetFriendList, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Post(FRIEND_REMOVE, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                FriendRemove(req, res);
            });
        http_server_.Post(FRIEND_ADD, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                FriendAdd(req, res);
            });
        http_server_.Post(FRIEND_ADD_PROCESS, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                FriendAddProcess(req, res);
            });
        http_server_.Get(FRIEND_SEARCH, 
            std::bind(&Gateway::FriendSearch, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Get(GET_CHAT_SESSION_LIST, 
            std::bind(&Gateway::GetChatSessionList, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Post(CHAT_SESSION_CREATE, 
            [this](const ::httplib::Request& req, ::httplib::Response& res){
                ChatSessionCreate(req, res);
            });
        http_server_.Get(GET_CHAT_SESSION_MEMBER, 
            std::bind(&Gateway::GetChatSessionMember, this, std::placeholders::_1, std::placeholders::_2));
        http_server_.Get(GET_PENDING_FRIEND_EVENT, 
            std::bind(&Gateway::GetPendingFriendEventList, this, std::placeholders::_1, std::placeholders::_2));
    }

    // 启动网关：HTTP 业务服务 + websocket 消息推送服务
    void run(const char* listen_ip, int port){
        http_server_thread_ = std::thread([this, listen_ip, port](){
            LOG_INFO("Gateway is listening {} on {}", listen_ip, port);
            http_server_.listen(listen_ip, port);
        });

        // websocket 服务器
        server_.set_access_channels(websocketpp::log::alevel::none); // 关闭日志打印
        server_.init_asio(); // 初始化底层网络框架

        // 设置操作回调
        server_.set_open_handler(std::bind(&Gateway::OnOpen, this, &server_, std::placeholders::_1));
        server_.set_close_handler(std::bind(&Gateway::OnClose, this, &server_, std::placeholders::_1));
        server_.set_fail_handler(std::bind(&Gateway::OnFail, this, &server_, std::placeholders::_1));
        server_.set_message_handler(std::bind(&Gateway::OnMessage, this, &server_, std::placeholders::_1, std::placeholders::_2));

        ws_server_thread_ = std::thread([this](){
            LOG_INFO("websocket server started on port: {}", ws_port_);
            server_.listen(ws_port_);
            server_.start_accept();
            server_.run();
        });
    }

    void stop(){
        http_server_.stop();
        if(http_server_thread_.joinable()){
            http_server_thread_.join();
        }
        LOG_INFO("Gateway is stopped");

        server_.stop();
        if(ws_server_thread_.joinable()){
            ws_server_thread_.join();
        }
        LOG_INFO("websocket server stopped");
    }
private:
    // 白名单接口不校验token
    const std::set<std::string> white_lists{
        GET_EMAIL_VERIFY_CODE,
        REGISTER,
        NICKNAME_LOGIN,
        EMAIL_LOGIN,
    };

    std::thread http_server_thread_;
    httplib::Server http_server_;
    std::shared_ptr<sw::redis::Redis> redis_template_;

    // 服务器通信相关
    std::shared_ptr<::brpc::ChannelOptions> options_;
    SvcDiscovery::Ptr svc_discovery_;

    // websocket 推送相关
    int ws_port_;
    server_t server_;
    Connection connection_; // 连接管理类
    std::thread ws_server_thread_;

    ///////////////////////////////
    // WebSocketServer
    ///////////////////////////////
    bool get_token(const std::string& raw, std::string& token){
        if(raw.empty()) return false;
        size_t pos = raw.find("token=");
        if (pos != std::string::npos) {
            token = raw.substr(pos + 6); // "token=" 长度是6
            size_t amp = token.find('&');
            if (amp != std::string::npos) {
                token = token.substr(0, amp);
            }
        }
        if(pos == std::string::npos) return false; // 没有或者不是 token= 开头
        return true;
    }

    // 连接打开
    void OnOpen(server_t *server, websocketpp::connection_hdl handle) {
        websocketpp::lib::error_code ec;
        auto conn = server_.get_con_from_hdl(handle, ec);
        if(ec && !conn){
            LOG_WARN("get_con_from_hdl failed: {}", ec.message());
            server->close(handle, websocketpp::close::status::policy_violation, errmsg::gateway::kWebSocketFailed);
            return;
        }
        auto uri = conn->get_uri();
        std::string raw = uri->get_query();
        std::string token;
        if(!get_token(raw, token)){
            server->close(handle, websocketpp::close::status::policy_violation, errmsg::kTokenInvalid);
            return;
        }
        if(!VerifyToken(token, redis_template_)){
            server->close(handle, websocketpp::close::status::policy_violation, errmsg::kTokenInvalid);
            return;
        }

        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            server->close(handle, websocketpp::close::status::policy_violation, errmsg::kTokenInvalid);
            return;
        }

        // 连接成功，将 user_id 放到连接管理类中
        connection_.AddConnection(user_id, conn);
        LOG_DEBUG("add connection: {}", user_id);

        return;
    }

    // 连接关闭
    void OnClose(server_t *server, websocketpp::connection_hdl handle) {
        // 我们前面就维护了 conn -> user_id + token 的映射
        auto conn = server_.get_con_from_hdl(handle);
        std::string user_id, token;
        if(!connection_.GetClient(conn, user_id, token)){
            return;
        }
        connection_.RemoveConnection(user_id);
        LOG_DEBUG("remove connection: {}", user_id);
    }

    // 连接断开
    void OnFail(server_t *server, websocketpp::connection_hdl handle) {
        auto conn = server_.get_con_from_hdl(handle);
        std::string user_id, token;
        if(!connection_.GetClient(conn, user_id, token)){
            LOG_ERROR("client not found (OnFail)");
            return;
        }
        connection_.RemoveConnection(user_id);
        LOG_DEBUG("remove connection: {}", user_id);
    }

    // 收到消息
    void OnMessage(server_t *server, websocketpp::connection_hdl handle,
                   message_t msg) {
        // websocket 服务器的在这里只做服务端到客户端的消息推送，不做逻辑，但是为了防止客户端因没有设置回调时发消息断开，这里还是注册
    }

    ///////////////////////////////
    // FileService
    ///////////////////////////////
    // GET /file/get_single?file_id=<文件ID>
    void GetSingleFile(const httplib::Request& req, httplib::Response& res){
        std::string file_id = req.get_param_value("file_id");
        if(file_id.empty()){
            res.status = 400;
            return;
        }

        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::file_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::file_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::file_service, ip);
            res.status = 502;
            return;
        }
        ::brpc::Controller cntl;
        ::linkchat::FileService_Stub stub(channel.get());

        ::linkchat::GetSingleFileReq request;
        ::linkchat::GetSingleFileRsp response;
        request.set_file_id(file_id);
        stub.GetSingleFile(&cntl, &request, &response, nullptr);

        if (cntl.Failed()){
            LOG_ERROR("Failed to get single file, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get single file response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /file/get_multi?file_ids=<id1>&file_ids=<id2>...（file_ids 重复出现，每个值是一个文件ID）
    void GetMultiFile(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::file_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::file_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::file_service, ip);
            res.status = 502;
                       return;
        }

        ::linkchat::GetMultiFileReq request;
        auto ids = req.params.equal_range("file_ids");
        for(auto it = ids.first; it != ids.second; ++it){
            request.add_file_ids(it->second);
        }
        if(request.file_ids_size() == 0){
            res.status = 400;
            return;
        }

        ::brpc::Controller cntl;
        ::linkchat::FileService_Stub stub(channel.get());
        ::linkchat::GetMultiFileRsp response;
        stub.GetMultiFile(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get multi file, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get multi file response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    ///////////////////////////////
    // MessageService
    ///////////////////////////////
    // GET /message/history?chat_session_id=<会话ID>&start_time=<起始时间戳>&over_time=<结束时间戳>
    // start_time、over_time 均为整型时间戳，返回该时间段内的消息
    void GetHistroyMsg(const ::httplib::Request& req, ::httplib::Response& res){
        // 与 brpc 服务器建立连接
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::message_storage_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::message_storage_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::message_storage_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::MsgStorageService_Stub stub(channel.get());
        ::linkchat::GetHistoryMsgReq request;
        ::linkchat::GetHistoryMsgRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 400;
            return;
        }
        request.set_token(token);
        std::string chat_session_id = req.get_param_value("chat_session_id");
        if(chat_session_id.empty()){
            res.status = 400;
            return;
        }
        request.set_chat_session_id(chat_session_id);
        std::string start_time_str = req.get_param_value("start_time");
        if(start_time_str.empty()){
            res.status = 400;
            return;
        }
        std::string over_time_str = req.get_param_value("over_time");
        if(over_time_str.empty()){
            res.status = 400;
            return;
        }
        long start_time, over_time;
        try{
            start_time = std::stol(start_time_str);
            over_time = std::stol(over_time_str);
        }
        catch(const std::exception& e){
            res.status = 400;
            return;
        }

        request.set_start_time(start_time);
        request.set_over_time(over_time);

        ::brpc::Controller cntl;
        stub.GetHistoryMsg(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get history msg, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get history msg response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /message/recent?chat_session_id=<会话ID>&msg_count=<条数>&cur_time=<当前时间戳>
    // msg_count 为整型，表示获取最近 N 条；cur_time 为整型时间戳
    void GetRecentMsg(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::message_storage_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::message_storage_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::message_storage_service, ip);
            res.status = 502;
            return;
        }

        ::brpc::Controller cntl;
        ::linkchat::MsgStorageService_Stub stub(channel.get());
        ::linkchat::GetRecentMsgReq request;
        ::linkchat::GetRecentMsgRsp response;

        std::string chat_session_id = req.get_param_value("chat_session_id");
        if(chat_session_id.empty()){
            res.status = 400;
            return;
        }
        request.set_chat_session_id(chat_session_id);
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 400;
            return;
        }
        request.set_token(token);
        std::string msg_count_str = req.get_param_value("msg_count");
        if(msg_count_str.empty()){
            res.status = 400;
            return;
        }
        int msg_count;
        try{
            msg_count = std::stoi(msg_count_str);
        }
        catch(const std::exception& e){
            res.status = 400;
            return;
        }
        request.set_msg_count(msg_count);
        long cur_time;
        try{
            cur_time = std::stol(req.get_param_value("cur_time"));
        }
        catch(const std::exception& e){
            res.status = 400;
            return;
        }
        request.set_cur_time(cur_time);

        stub.GetRecentMsg(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get recent msg, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get recent msg response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /message/search?chat_session_id=<会话ID>&search_key=<消息搜索关键字>
    void MsgSearch(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::message_storage_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::message_storage_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::message_storage_service, ip);
            res.status = 502;
            return;
        }

        ::brpc::Controller cntl;
        ::linkchat::MsgStorageService_Stub stub(channel.get());
        ::linkchat::MsgSearchReq request;
        ::linkchat::MsgSearchRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 400;
            return;
        }
        request.set_token(token);
        std::string chat_session_id = req.get_param_value("chat_session_id");
        if(chat_session_id.empty()){
            res.status = 400;
            return;
        }
        request.set_chat_session_id(chat_session_id);
        std::string search_key = req.get_param_value("search_key");
        if(search_key.empty()){
            res.status = 400;
            return;
        }
        request.set_search_key(search_key);

        stub.MsgSearch(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to search msg, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize msg search response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    ///////////////////////////////
    // UserService
    ///////////////////////////////
    // POST /register，body 为 application/json：
    // {"nickname": "昵称", "password": "密码", "email": "邮箱", "verify_code": "邮箱验证码"}
    void Register(const ::httplib::Request& req, ::httplib::Response& res){
        // 与 brpc 服务器建立连接
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }
        ::linkchat::UserService_Stub stub(channel.get());

        ::linkchat::UserRegisterReq request;
        ::linkchat::UserRegisterRsp response;
        std::string body = req.body;
        auto status = ::google::protobuf::util::JsonStringToMessage(body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }

        ::brpc::Controller cntl;
        stub.UserRegister(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to register, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize register response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /login/mail_verify_code?email=<邮箱>
    void GetEmailVerifyCode(const httplib::Request& req, httplib::Response& res){
        // 与 brpc 服务器建立连接
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }
        ::linkchat::UserService_Stub stub(channel.get());

        std::string email = req.get_param_value("email"); // 获取路径参数

        // 调用用户服务获取用户信息
        ::brpc::Controller cntl;
        ::linkchat::EmailVerifyCodeReq request;
        request.set_email(email);
        ::linkchat::EmailVerifyCodeRsp response;
        stub.GetEmailVerifyCode(&cntl, &request, &response, nullptr);

        if (cntl.Failed()){
            LOG_ERROR("Failed to get email verify code, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize email verify code response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /login/nickname，body 为 application/json：
    // {"nickname": "昵称", "password": "密码"}
    void NicknameLogin(const ::httplib::Request& req, ::httplib::Response& res){
        // 与 brpc 服务器建立连接
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }
        ::linkchat::UserService_Stub stub(channel.get());
        
        ::linkchat::NicknameLoginReq request;
        ::linkchat::NicknameLoginRsp response;
        std::string body = req.body;
        auto status = ::google::protobuf::util::JsonStringToMessage(body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }

        ::brpc::Controller cntl;
        stub.NicknameLogin(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to nickname login, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize nickname login response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /login/email，body 为 application/json：
    // {"email": "邮箱", "verify_code": "邮箱验证码"}
    void EmailLogin(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }
        ::linkchat::UserService_Stub stub(channel.get());
        
        ::linkchat::EmailLoginReq request;
        ::linkchat::EmailLoginRsp response;
        std::string body = req.body;
        auto status = ::google::protobuf::util::JsonStringToMessage(body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }

        ::brpc::Controller cntl;
        stub.EmailLogin(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to email login, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize email login response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // 这个方法对应的后端接口，只接受 user_id
    // GET /user/info，无需参数，当前用户身份从 Authorization token 中解析
    void GetUserInfo(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::GetUserInfoReq request;
        ::linkchat::GetUserInfoRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        request.set_user_id(user_id);

        ::brpc::Controller cntl;
        stub.GetUserInfo(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get user info, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get info response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /user/multi_info?users_id=<id1>&users_id=<id2>...（users_id 重复出现，每个值是一个用户ID）
    void GetMultiUserInfo(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::GetMultiUserInfoReq request;
        ::linkchat::GetMultiUserInfoRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            std::cout << "token empty" << std::endl;
            res.status = 401;
            return;
        }
        auto ids = req.params.equal_range("users_id");
        for(auto it = ids.first; it != ids.second; ++it){
            request.add_users_id(it->second);
        }
        if(request.users_id_size() == 0){
            res.status = 400;
            return;
        }
        ::brpc::Controller cntl;
        stub.GetMultiUserInfo(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get multi user info, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /user/avatar/set，body 为 multipart/form-data：
    // 表单字段 avatar = 头像图片文件（二进制内容）
    void SetUserAvatar(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::SetUserAvatarReq request;
        ::linkchat::SetUserAvatarRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        request.set_token(token);
        if(!req.has_file("avatar")){
            res.status = 400;
            return;
        }
        const auto& avatar_content = req.get_file_value("avatar");
        std::string avatar = avatar_content.content;
        request.set_avatar(avatar);
        ::brpc::Controller cntl;
        stub.SetUserAvatar(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to set user avatar, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /user/description/set，参数在 query string（不在 body）：
    // /user/description/set?description=<个人描述>
    void SetUserDescription(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::SetUserDescriptionReq request;
        ::linkchat::SetUserDescriptionRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        request.set_token(token);
        std::string description = req.get_param_value("description");
        if(description.empty()){
            res.status = 400;
            return;
        }
        request.set_description(description);
        ::brpc::Controller cntl;
        stub.SetUserDescription(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to set user description, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /user/nickname/set，参数在 query string（不在 body）：
    // /user/nickname/set?nickname=<新昵称>
    void SetUserNickname(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::user_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::user_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::user_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::UserService_Stub stub(channel.get());
        ::linkchat::SetUserNicknameReq request;
        ::linkchat::SetUserNicknameRsp response;
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        request.set_token(token);
        std::string nickname = req.get_param_value("nickname");
        if(nickname.empty()){
            res.status = 400;
            return;
        }
        request.set_nickname(nickname);
        ::brpc::Controller cntl;
        stub.SetUserNickname(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to set user nickname, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /transmite/publish，body 为 application/json（token 由网关从 Authorization 头注入，body 不用带）：
    // 文字消息：
    // {"chat_session_id": "会话ID", "message": {"message_type": "STRING", "string_message": {"content": "文字内容"}}}
    // 文件消息：
    // {"chat_session_id": "会话ID", "message": {"message_type": "FILE",
    //   "file_message": {"file_name": "文件名", "file_size": 123, "file_content": "文件base64内容"}}}
    void NewMessage(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::transmite_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::transmite_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::transmite_service, ip);
            res.status = 502;
            return;
        }

        ::brpc::Controller cntl;
        ::linkchat::MsgTransmitService_Stub stub(channel.get());
        ::linkchat::NewMessageReq request;
        ::linkchat::GetTransmitTargetRsp response;
        auto status = ::google::protobuf::util::JsonStringToMessage(req.body, &request);
        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        request.set_token(token);
        if(!status.ok()){
            res.status = 400;
            return;
        }
        stub.NewMessage(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to publish message, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }

        // 现在还没有搭建网关的主动推送部分，这里只把 targets_id 打出来
        std::vector<std::string> targets_id;
        for(auto& target: response.target_id_list()){
            targets_id.push_back(target);
        }
        LOG_INFO("targets_id's size: {}", targets_id.size());
        for(auto& target: targets_id){
            LOG_INFO("target_id: {}", target);
        }
        LOG_INFO("========================");

        ::linkchat::NewMessageRsp rsp;
        rsp.set_success(response.success());
        rsp.set_errmsg(response.errmsg());
        std::string content;
        ::google::protobuf::util::MessageToJsonString(rsp, &content);
        if(content == ""){
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    ///////////////////////////////
    // FriendService
    ///////////////////////////////
    // GET /friend/list，无需参数，user_id 从 Authorization token 中解析
    void GetFriendList(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::GetFriendListReq request;
        ::linkchat::GetFriendListRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        request.set_user_id(user_id);

        ::brpc::Controller cntl;
        stub.GetFriendList(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get friend list, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get friend list response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /friend/remove，body 为 application/json：
    // {"peer_id": "要删除的好友用户ID"}
    void FriendRemove(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::FriendRemoveReq request;
        ::linkchat::FriendRemoveRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        auto status = ::google::protobuf::util::JsonStringToMessage(req.body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }
        if(request.peer_id().empty()){
            res.status = 400;
            return;
        }
        request.set_user_id(user_id);

        ::brpc::Controller cntl;
        stub.FriendRemove(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to friend remove, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize friend remove response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /friend/add，body 为 application/json：
    // {"respondent_id": "被申请人用户ID"}
    void FriendAdd(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::FriendAddReq request;
        ::linkchat::FriendAddRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        auto status = ::google::protobuf::util::JsonStringToMessage(req.body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }
        if(request.respondent_id().empty()){
            res.status = 400;
            return;
        }
        request.set_user_id(user_id);

        ::brpc::Controller cntl;
        stub.FriendAdd(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to friend add, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize friend add response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /friend/add_process，body 为 application/json：
    // {"notify_event_id": "好友申请事件ID", "agree": true, "apply_user_id": "申请人用户ID"}
    // agree 为 bool：true 同意 / false 拒绝；被申请人（当前用户）从 token 中解析，body 不用带
    void FriendAddProcess(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::FriendAddProcessReq request;
        ::linkchat::FriendAddProcessRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        auto status = ::google::protobuf::util::JsonStringToMessage(req.body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }
        if(request.notify_event_id().empty() || request.apply_user_id().empty()){
            res.status = 400;
            return;
        }
        // 被申请人从 token 中获取
        request.set_user_id(user_id);

        ::brpc::Controller cntl;
        stub.FriendAddProcess(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to friend add process, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize friend add process response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /friend/search?search_key=<昵称模糊匹配关键字>&count=<返回数量>
    // count 为整型，表示最多返回几个用户
    void FriendSearch(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::FriendSearchReq request;
        ::linkchat::FriendSearchRsp response;

        std::string search_key = req.get_param_value("search_key");
        if(search_key.empty()){
            res.status = 400;
            return;
        }
        request.set_search_key(search_key);
        std::string count_str = req.get_param_value("count");
        if(count_str.empty()){
            res.status = 400;
            return;
        }
        int count;
        try{
            count = std::stoi(count_str);
        }
        catch(const std::exception& e){
            res.status = 400;
            return;
        }
        request.set_count(count);

        ::brpc::Controller cntl;
        stub.FriendSearch(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to friend search, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize friend search response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /friend/chat_session/list，无需参数，user_id 从 Authorization token 中解析
    void GetChatSessionList(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::GetChatSessionListReq request;
        ::linkchat::GetChatSessionListRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        request.set_user_id(user_id);

        ::brpc::Controller cntl;
        stub.GetChatSessionList(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get chat session list, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get chat session list response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // POST /friend/chat_session/create，body 为 application/json：
    // {"chat_session_name": "群聊名称", "member_id_list": ["用户ID1", "用户ID2"]}
    // 创建者（当前用户）从 token 中解析，body 不用带，"member_id_list" 中不需要带创建者 id
    void ChatSessionCreate(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::ChatSessionCreateReq request;
        ::linkchat::ChatSessionCreateRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        auto status = ::google::protobuf::util::JsonStringToMessage(req.body, &request);
        if(!status.ok()){
            res.status = 400;
            return;
        }
        if(request.chat_session_name().empty() || request.member_id_list_size() == 0){
            res.status = 400;
            return;
        }
        // 创建者从 token 中获取
        request.set_user_id(user_id);

        ::brpc::Controller cntl;
        stub.ChatSessionCreate(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to chat session create, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize chat session create response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /friend/chat_session/member?chat_session_id=<会话ID>
    void GetChatSessionMember(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::GetChatSessionMemberReq request;
        ::linkchat::GetChatSessionMemberRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        request.set_user_id(user_id);
        std::string chat_session_id = req.get_param_value("chat_session_id");
        if(chat_session_id.empty()){
            res.status = 400;
            return;
        }
        request.set_chat_session_id(chat_session_id);

        ::brpc::Controller cntl;
        stub.GetChatSessionMember(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get chat session member, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get chat session member response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }

    // GET /friend/pending_event，无需参数，user_id 从 Authorization token 中解析
    void GetPendingFriendEventList(const ::httplib::Request& req, ::httplib::Response& res){
        std::string ip;
        auto channel = svc_discovery_->GetSvcChannel(service_name::friend_service, ip);
        if(ip == ""){
            LOG_ERROR("No nodes available for {}", service_name::friend_service);
            res.status = 502;
            return;
        }
        if(channel->Init(ip.c_str(), options_.get()) != 0){
            LOG_ERROR("Failed to init brpc channel to {}, ip {}", service_name::friend_service, ip);
            res.status = 502;
            return;
        }

        ::linkchat::FriendService_Stub stub(channel.get());
        ::linkchat::GetPendingFriendEventListReq request;
        ::linkchat::GetPendingFriendEventListRsp response;

        std::string token = req.get_header_value("Authorization");
        if(token.empty()){
            res.status = 401;
            return;
        }
        std::string user_id;
        if(!JwtParse(token, "user_id", user_id)){
            res.status = 401;
            return;
        }
        request.set_user_id(user_id);

        ::brpc::Controller cntl;
        stub.GetPendingFriendEventList(&cntl, &request, &response, nullptr);
        if (cntl.Failed()){
            LOG_ERROR("Failed to get pending friend event list, error: {}", cntl.ErrorText());
            res.status = 502;
            return;
        }
        std::string content;
        ::google::protobuf::util::MessageToJsonString(response, &content);
        if(content == ""){
            LOG_ERROR("Failed to serialize get pending friend event list response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }
};