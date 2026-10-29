#include <gflags/gflags.h>

#include "adapter/logging/logger.hpp"
#include "adapter/mail/mail_sender.hpp"

DEFINE_string(smtp_url, "", "smtp 服务器 url");
DEFINE_string(smtp_account, "", "smtp 账号名");
DEFINE_string(smtp_password, "", "smtp 认证密码");

int main(int argc, char* argv[]){
    google::SetCommandLineOption("flagfile", "../settings.conf");
    std::cout << FLAGS_smtp_url << " " << FLAGS_smtp_account << " " << FLAGS_smtp_password << std::endl;
    google::ParseCommandLineFlags(&argc, &argv, true);
    init_logger();

    mail::MailMessage message{"1393265226@qq.com", "25300120122@m.fudan.edu.cn", "test", "test email sending"};
    mail::send_mail(FLAGS_smtp_url, FLAGS_smtp_account, FLAGS_smtp_password, message);
    return 0;
}