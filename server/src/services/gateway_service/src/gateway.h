#include <brpc/controller.h>
#include <httplib.h>
#include <string>

#include "user.pb.h"
#include "file.pb.h"
#include "message.pb.h"
#include "logging/logger.h"

class Gateway{
public:
    #define GET_EMAIL_VERIFY_CODE "/get_email_verify_code/:email"

    Gateway(){

    }

    void run(const char* listen_ip, int port){
        http_server_.listen(listen_ip, port);
    }
private:
    httplib::Server http_server_;

    // 各个服务器 stub
    brpc::Controller cntl_;
    std::shared_ptr<::linkchat::FileService_Stub> stub_;
    std::shared_ptr<::linkchat::MsgStorageService_Stub> message_stub_;
    std::shared_ptr<::linkchat::UserService_Stub> user_stub_;

    void GetEmailVerifyCode(const httplib::Request& req, httplib::Response& res){
        std::string email = req.get_param_value("email"); // 获取路径参数

        // 调用用户服务获取用户信息
        ::linkchat::EmailVerifyCodeReq request;
        request.set_email(email);
        ::linkchat::EmailVerifyCodeRsp response;
        user_stub_->GetEmailVerifyCode(&cntl_, &request, &response, nullptr);

        if (cntl_.Failed()){
            res.status = 502;
            return;
        }
        std::string content = response.SerializeAsString();
        if(content == ""){
            LOG_ERROR("Failed to serialize email verify code response");
            res.status = 500;
            return;
        }

        res.status = 200;
        res.set_content(content, "application/json");
    }
};