#include <gflags/gflags.h>

#include "adapter/oss/oss_client.h"
#include "util/file_util.h"

DEFINE_string(aliyun_access_id, "", "AccessId");
DEFINE_string(aliyun_access_key, "", "AccessKey");
DEFINE_string(aliyun_endpoint, "", "Endpoint");
DEFINE_string(aliyun_region, "", "Region");
DEFINE_string(aliyun_bucket_name, "", "Bucket Name");

int main(int argc, char* argv[]) {
    google::SetCommandLineOption("flagfile", "../settings.conf");
    google::ParseCommandLineFlags(&argc, &argv, true);
    OssClient oss(FLAGS_aliyun_endpoint, FLAGS_aliyun_region, FLAGS_aliyun_bucket_name,
        FLAGS_aliyun_access_id, FLAGS_aliyun_access_key);
    oss.Init();

    std::string binary_content = file::ReadFileToString("../logo.png");
    oss.Upload(binary_content, "test_upload1.png");
    //std::cout << oss.PresignedDownload("NP0222_spr_19.png") << std::endl;
    return 0;
}
