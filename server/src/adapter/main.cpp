#include <iostream>
#include <jsoncpp/json/json.h>

#include "logging/logger.hpp"
#include "json/json_codec.hpp"

int main() {
    init_logger(false);

    Json::Value obj;
    obj["name"] = "John";
    obj["age"] = 20;
    obj["gender"] = "Male";
    obj["skills"].append("C++");
    obj["skills"].append("Java");
    obj["skills"].append("Python");

    Json::Value json;
    std::string str = JsonCodec::serialize(json);
    json = JsonCodec::deserialize(str);

    std::cout << obj["name"] << std::endl;
    std::cout << obj["age"] << std::endl;
    std::cout << obj["gender"] << std::endl;
    std::cout << obj["skills"] << std::endl;
    return 0;
}