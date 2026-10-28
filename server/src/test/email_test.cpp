#include <gflags/gflags.h>

#include "adapter/logging/logger.hpp"
#include "adapter/email/mail_sender.hpp"

DEFINE_string(smtp_url, "", "smtp 服务器 url");
DEFINE_string(smtp_account, "", "smtp 账号名");
DEFINE_string(smtp_password, "", "smtp 认证密码");

int main(int argc, char* argv[]){
    google::SetCommandLineOption("flagfile", "../settings.conf");
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger();

    mail::MailMessage message{"", "", "test", "test email sending"};
    mail::send_mail(FLAGS_smtp_url, FLAGS_smtp_account, FLAGS_smtp_password, message);
    return 0;
}