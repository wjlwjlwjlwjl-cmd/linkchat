#include <gflags/gflags.h>

#include "adapter/logging/logger.h"
#include "adapter/mail/mail_sender.h"

DEFINE_string(smtp_url, "", "SMTP server url");
DEFINE_string(smtp_account, "", "SMTP account");
DEFINE_string(smtp_password, "", "SMTP password");

int main(int argc, char* argv[]) {
    google::SetCommandLineOption("flagfile", "../settings.conf");
    std::cout << FLAGS_smtp_url << " " << FLAGS_smtp_account << " " << FLAGS_smtp_password << std::endl;
    google::ParseCommandLineFlags(&argc, &argv, true);
    InitLogger();

    mail::MailMessage message{"1393265226@qq.com", "25300120122@m.fudan.edu.cn", "test", "test email sending"};
    mail::SendMail(FLAGS_smtp_url, FLAGS_smtp_account, FLAGS_smtp_password, message);
    return 0;
}
