#include <elasticlient/client.h>
#include <cpr/response.h>
#include <iostream>

const std::string kIndexName = "message";
const std::string kDocType = "_doc";
const std::string kId = "";

void CreateIndex(elasticlient::Client& client) {
    std::string body(
        R"(
        {
            "settings":{
                "analysis":{
                    "analyzer": {
                        "ik_max": {
                            "type": "custom",
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
    auto response = client.index(kIndexName, kDocType, kId, body);
    if (response.status_code < 200 || response.status_code >= 300) {
        std::cout << "Create es index failed, status: " << response.status_code << std::endl;
        std::cout << response.text << std::endl;
        return;
    }
    std::cout << "Create es index success: " << response.text << std::endl;
}

void AddData(elasticlient::Client& client) {
    const std::string body(
        R"(
            {
                "id": 1,
                "name": "john",
                "introduction": "this is an introduction for test"
            }
        )"
    );
    auto response = client.index(kIndexName, kDocType, kId, body);
    if (response.status_code < 200 || response.status_code >= 300) {
        std::cout << "Add es data failed, status: " << response.status_code << std::endl;
        std::cout << response.text << std::endl;
        return;
    }
    std::cout << "Add es data success: " << response.text << std::endl;
}

void RemoveData(elasticlient::Client& client) {
    auto response = client.remove(kIndexName, kDocType, "6jvFgKEBpRJXg3MEozEb");
    if (response.status_code < 200 || response.status_code >= 300) {
        std::cout << "Remove es data failed, status: " << response.status_code << std::endl;
        std::cout << response.text << std::endl;
        return;
    }
    std::cout << "Remove es data success: " << response.text << std::endl;
}

void GetData(elasticlient::Client& client) {
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
    auto response = client.search(kIndexName, kDocType, query);
    if (response.status_code < 200 || response.status_code >= 300) {
        std::cout << "Get es data failed, status: " << response.status_code << std::endl;
        std::cout << response.text << std::endl;
        return;
    }
    std::cout << "Get es data success: " << response.text << std::endl;
}

int main() {
    elasticlient::Client client({"http://elastic:123456@elasticsearch-service:9200/"});
    //CreateIndex(client);
    //AddData(client);
    GetData(client);
    return 0;
}
