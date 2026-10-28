#include <elasticlient/client.h>
#include <cpr/response.h>
#include <iostream>

const std::string index_name = "message";
const std::string doc_type = "_doc";
const std::string id = "";

void create_index(elasticlient::Client& client){
    std::string body(
        R"(
        {
            "settings":{
                "analysis":{
                    "analyzer": {
                        "ik_max": {
                            "tokenizer": "ik_max_word"
                        }
                    }
                }
            },
            "mappings":{
                "dynamic": false,
                "properties": {
                    "id": {
                        "type": "keyword"
                    },
                    "name": {
                        "type":"text",
                        "analyzer": "ikmax"
                    },
                    "introduction": {
                        "type": "text",
                        "analyzer": "ikmax"
                    }
                }
            }
        }
        )"
    );
    auto resp = client.index(index_name, doc_type, id, body);
    if(resp.status_code < 200 || resp.status_code >= 300){
        std::cout << "es 索引创建失败: " << resp.status_code << std::endl;
        std::cout << resp.text << std::endl;
        return;
    }
    std::cout << "es 索引创建成功：" << resp.text << std::endl;
}

void add_data(elasticlient::Client& client){
    const std::string body(
        R"(
            {
                "id": 1,
                "name": "john",
                "introduction": "this is an introduction for test"
            }
        )"
    );
    auto resp = client.index(index_name, doc_type, id, body);
    if(resp.status_code < 200 || resp.status_code >= 300){
        std::cout << "es 数据插入失败: " << resp.status_code << std::endl;
        std::cout << resp.text << std::endl;
        return;
    }
    std::cout << "es 数据插入成功：" << resp.text << std::endl;
}

void remove_data(elasticlient::Client& client){
    auto resp = client.remove(index_name, doc_type, "6jvFgKEBpRJXg3MEozEb");
    if(resp.status_code < 200 || resp.status_code >= 300){
        std::cout << "es 数据删除失败: " << resp.status_code << std::endl;
        std::cout << resp.text << std::endl;
        return;
    }
    std::cout << "es 数据删除成功：" << resp.text << std::endl;
}

void get_data(elasticlient::Client& client){
    const std::string query(
        R"(
            {
                "query": {
                    "match_all":{

                    }
                }
            }
        )"
    );
    auto resp = client.search(index_name, doc_type, query);
    if(resp.status_code < 200 || resp.status_code >= 300){
        std::cout << "es 获取数据失败: " << resp.status_code << std::endl;
        std::cout << resp.text << std::endl;
        return;
    }
    std::cout << "es 获取数据成功：" << resp.text << std::endl;
}

int main(){
    elasticlient::Client client({"http://elastic:123456@elasticsearch-service:9200/"});
    //create_index(client);
    //add_data(client);
    get_data(client);
    return 0;
}