#pragma once

#include "common_dto.h"
#include "file_info.h"
#include "message_info.h"

// 下载单个文件结果
struct GetSingleFileDTO : public CommonDTO {
    FileDownloadData file_data; //file_id, presigned_url
};

// 上传单个文件结果
struct PutSingleFileDTO : public CommonDTO {
    FileMessageInfo file_info;
};
