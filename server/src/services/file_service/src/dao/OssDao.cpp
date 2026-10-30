#include "OssDao.h"
#include "file_util.h"

class FileServiceImpl: public FileService{
public:
    FileServiceImpl(const std::string& endpoint, const std::string& region, const std::string& bucket_name,
        const std::string& access_id, const std::string& access_key, int64_t url_expiration = 900)
        : _client(std::make_shared<OssClient>(endpoint, region, bucket_name, access_id, access_id, url_expiration))
    {
        _client->Init();
    }    

    linkchat::FileDownloadData GetSingleFile(const std::string& file_id){
        std::string presigned_url = _client->PresignedDownload(file_id);

        linkchat::FileDownloadData file_download_data;
        file_download_data.set_file_id(file_id);
        file_download_data.set_presigned_url(presigned_url);

        return file_download_data;
    }

    std::unordered_map<std::string, linkchat::FileDownloadData> GetMultipleFile(const std::vector<std::string>& file_ids){
        std::unordered_map<std::string, linkchat::FileDownloadData> ret;
        for(auto& file_id: file_ids){
            std::string presigned_url = _client->PresignedDownload(file_id);
            linkchat::FileDownloadData file_data;
            file_data.set_file_id(file_id);
            file_data.set_presigned_url(presigned_url);
            ret[file_id] = file_data;
        }
        return ret;
    }

    linkchat::FileMessageInfo PutSingleFile(const linkchat::FileUploadData& upload_data){
        linkchat::FileMessageInfo ret;

        std::string file_name = upload_data.file_name();
        int64_t file_size = upload_data.file_size();
        std::string file_content = upload_data.file_content();

        if(file_size > 20 * 1024 * 1024){
            return ret;
        }
        _client->Upload(file_content, file_name);

        std::string file_id = "";
    }
    std::vector<linkchat::FileMessageInfo> PutMultipleFile(const std::vector<linkchat::FileUploadData>&);
private:
    OssClient::Ptr _client;
};