#pragma once

#include <curl/curl.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <string_view>

#include "adapter/logging/logger.h"

namespace mail {

    //释放 CURL* 句柄资源
    struct CurlDeleter {
        void operator()(CURL* curl) const noexcept { if (curl) curl_easy_cleanup(curl); }
    };

    //释放libcurl内部单链表结构（这里我们用来存放发送人）
    struct SlistDeleter {
        void operator()(curl_slist* list) const noexcept { if (list) curl_slist_free_all(list); }
    };

    using CurlHandle = std::unique_ptr<CURL, CurlDeleter>; //unique_ptr 第二个模版参数为删除器，要求是可调用对象即可
    using SlistHandle = std::unique_ptr<curl_slist, SlistDeleter>;

    struct MailMessage {
        std::string from;
        std::string to;
        std::string subject;
        std::string body;
    };

    inline std::string BuildPayload(const MailMessage& mail) {
        const std::string from = mail.from;
        const std::string to = mail.to;
        const std::string subject = mail.subject;

        std::ostringstream oss;
        oss << "From: " << from << "\r\n"
            << "To: " << to << "\r\n"
            << "Subject: " << subject << "\r\n"
            << "\r\n"                       // 头部与正文的分隔，必须有
            << mail.body;
        return oss.str();
    }

    namespace detail {
        struct UploadContext {
            std::string_view data;   // 不拥有数据，指向 payload
            std::size_t offset = 0;
        };

        inline std::size_t ReadCallback(char* ptr, std::size_t size, std::size_t nmemb, void* userp) {
            auto* context = static_cast<UploadContext*>(userp);
            const std::size_t room = size * nmemb;
            const std::size_t remaining = context->data.size() - context->offset;
            if (remaining == 0) return 0;                 // 数据发完，通知 libcurl 结束
            const std::size_t len = std::min(room, remaining);
            std::memcpy(ptr, context->data.data() + context->offset, len);
            context->offset += len;
            return len;
        }
    }

    inline bool SendMail(const std::string& host, const std::string& user, const std::string& pass,
        const MailMessage& mail) {
        static const bool global_ok = [] {
            return curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK; //初始化 libcurl 底层系统
        }();
        if (!global_ok) {
            LOG_ERROR("failed to init underlying system of libcurl");
            return false;
        }

        CurlHandle curl(curl_easy_init()); //创建 libcurl 账号、url、smtp 会话等依赖的核心句柄 CURL*
        if (!curl) {
            std::cerr << "curl_easy_init failed\n";
            return false;
        }

        const std::string payload = BuildPayload(mail);
        detail::UploadContext context{ payload };
        SlistHandle rcpt(curl_slist_append(nullptr, mail.to.c_str())); //slist 是 libcurl 中的一个单链表结构，这里新建一个来存放联系人

        //完成会话配置
        curl_easy_setopt(curl.get(), CURLOPT_URL, host.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_USERNAME, user.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_PASSWORD, pass.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl.get(), CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");
        curl_easy_setopt(curl.get(), CURLOPT_MAIL_FROM, mail.from.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_MAIL_RCPT, rcpt.get());
        curl_easy_setopt(curl.get(), CURLOPT_READFUNCTION, detail::ReadCallback);
        curl_easy_setopt(curl.get(), CURLOPT_READDATA, &context);
        curl_easy_setopt(curl.get(), CURLOPT_UPLOAD, 1L);

        //发起会话，自动进行DNS 解析、TCP 连接，同步阻塞，反复调用 ReadCallback
        const CURLcode res = curl_easy_perform(curl.get());
        if (res != CURLE_OK) {
            LOG_ERROR("send failed: {}", curl_easy_strerror(res)); //将libcurl错误码转换为刻度的错误信息
            return false;
        }
        return true;
    }

}
