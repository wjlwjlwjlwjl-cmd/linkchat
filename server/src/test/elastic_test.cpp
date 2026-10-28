#include <iostream>

#include "adapter/elastic/elastic_client.hpp"

int main(){
    init_logger();
    ElasticClient::PropType props;
    props.insert({"user_id", {{"type", "keyword"}}});
    props.insert({"message", {{"type", "text"}, {"analyzer", "ikmax"}}});

    ElasticClient es_client("test_index");
    //es_client.CreateIndex(props);

    //ElasticClient::DataType data = {{"user_id", std::to_string(1)}, {"message", "this is a message for test"}};
    //es_client.AddData(data);

    auto ret = es_client.SelectTerm("user_id", "1");
    std::cout << JsonCodec::serialize(ret) << std::endl;
    std::cout << "===========" << std::endl;
    auto ret1 = es_client.SelectMatch("message", "this");
    std::cout << JsonCodec::serialize(ret1) << std::endl;

    es_client.Remove(es_client.Id(ret1, 0));
    return 0;
}