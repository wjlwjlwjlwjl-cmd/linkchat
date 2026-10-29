#include <iostream>

#include "adapter/elastic/elastic_client.h"

int main() {
    InitLogger();
    ElasticClient::PropType properties;
    properties.insert({"user_id", {{"type", "keyword"}}});
    properties.insert({"message", {{"type", "text"}, {"analyzer", "ikmax"}}});

    ElasticClient es_client("test_index");
    //es_client.CreateIndex(properties);

    //ElasticClient::DataType data = {{"user_id", std::to_string(1)}, {"message", "this is a message for test"}};
    //es_client.AddData(data);

    auto result = es_client.SelectTerm("user_id", "1");
    std::cout << JsonCodec::Serialize(result) << std::endl;
    std::cout << "===========" << std::endl;
    auto match_result = es_client.SelectMatch("message", "this");
    std::cout << JsonCodec::Serialize(match_result) << std::endl;

    es_client.Remove(es_client.Id(match_result, 0));
    return 0;
}
