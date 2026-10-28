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

#include "adapter/logging/logger.hpp"

namespace mail {

    //释放 CURL* 句柄资源
    struct CurlDeleter {
        void operator()(CURL* c) const noexcept { if (c) curl_easy_cleanup(c); }
    };

    //释放libcurl内部单链表结构（这里我们用来存放发送人）
    struct SlistDeleter {
        void operator()(curl_slist* s) const noexcept { if (s) curl_slist_free_all(s); }
    };

    using CurlHandle  = std::unique_ptr<CURL, CurlDeleter>; //unique_ptr 第二个模版参数为删除器，要求是可调用对象即可
    using SlistHandle = std::unique_ptr<curl_slist, SlistDeleter>;

    struct MailMessage {
        std::string from;
        std::string to;
        std::string subject;
        std::string body;
    };

    // ---------- 纯文本邮件的报文构造（RFC 5322 + RFC 2045/2047） ----------
    inline std::string build_payload(const MailMessage& mail) {
        const std::string from    = mail.from;
        const std::string to      = mail.to;
        const std::string subject = mail.subject;

        std::ostringstream oss;
        oss << "From: "                     << from                        << "\r\n"
            << "To: "                       << to                          << "\r\n"
            << "Subject: "                  << subject                     << "\r\n"
            << "\r\n"                       // 头部与正文的分隔，必须有
            << mail.body;
        return oss.str();
    }

    namespace detail {
        struct UploadCtx {
            std::string_view data;   // 不拥有数据，指向 payload
            std::size_t offset = 0;
        };

        inline std::size_t read_cb(char* ptr, std::size_t size, std::size_t nmemb, void* userp) {
            auto* ctx = static_cast<UploadCtx*>(userp);
            const std::size_t room = size * nmemb;
            const std::size_t remaining = ctx->data.size() - ctx->offset;
            if (remaining == 0) return 0;                 // 数据发完，通知 libcurl 结束
            const std::size_t len = std::min(room, remaining);
            std::memcpy(ptr, ctx->data.data() + ctx->offset, len);
            ctx->offset += len;
            return len;
        }
    }

    inline bool send_mail(const std::string& host, const std::string& user, const std::string& pass,
                        const MailMessage& mail) {
        static const bool global_ok = [] {
            return curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK; //初始化 libcurl 底层系统
        }();
        if (!global_ok) {
            ERR("failed to init underlying system of libcurl");
            return false;
        }

        CurlHandle curl(curl_easy_init()); //创建 libcurl 账号、url、smtp 会话等依赖的核心句柄 CURL*
        if (!curl) {
            std::cerr << "curl_easy_init failed\n";
            return false;
        }

        const std::string payload = build_payload(mail);
        detail::UploadCtx ctx{ payload };
        SlistHandle rcpt(curl_slist_append(nullptr, mail.to.c_str())); //slist 是 libcurl 中的一个单链表结构，这里新建一个来存放联系人

        //完成会话配置
        curl_easy_setopt(curl.get(), CURLOPT_URL,           host.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_USERNAME,      user.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_PASSWORD,      pass.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_USE_SSL,       CURLUSESSL_ALL);
        curl_easy_setopt(curl.get(), CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");
        curl_easy_setopt(curl.get(), CURLOPT_MAIL_FROM,     mail.from.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_MAIL_RCPT,     rcpt.get());
        curl_easy_setopt(curl.get(), CURLOPT_READFUNCTION,  detail::read_cb);
        curl_easy_setopt(curl.get(), CURLOPT_READDATA,      &ctx);
        curl_easy_setopt(curl.get(), CURLOPT_UPLOAD,        1L);

        //发起会话，同步阻塞，反复调用 read_cb
        const CURLcode res = curl_easy_perform(curl.get());
        if (res != CURLE_OK) {
            ERR("send failed: {}", curl_easy_strerror(res)); //将libcurl错误码转换为刻度的错误信息
            return false;
        }
        return true;
    }

}