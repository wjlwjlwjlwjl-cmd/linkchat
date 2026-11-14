#pragma once

#include <string>

struct FileDownloadData {
    std::string file_id;
    std::string presigned_url;
};

struct FileUploadData {
    std::string file_name; //文件名称
    std::string file_content; //文件数据
};
