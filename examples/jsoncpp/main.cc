#include <iostream>
#include <jsoncpp/json/json.h>

std::string Serialize() {
    Json::Value object;
    object["name"] = "John";
    object["age"] = 20;
    object["gender"] = "Male";
    object["skills"].append("C++");
    object["skills"].append("Java");
    object["skills"].append("Python");

    auto builder = Json::StreamWriterBuilder();
    builder["indentation"] = "  ";           // 缩进改为 2 个空格（默认是制表符 \t）
    builder["commentStyle"] = "None";        // 不输出注释，避免干扰
    builder["enableYAMLCompatibility"] = true; // 在冒号后加空格，例如 "key": "value"

    auto writer = builder.newStreamWriter();
    std::stringstream ss;
    try {
        writer->write(object, &ss);
    }
    catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    std::cout << ss.str() << std::endl;
    return ss.str();
}

Json::Value Deserialize(const std::string& str) {
    auto builder = Json::CharReaderBuilder();
    auto reader = builder.newCharReader();
    Json::Value object;
    std::string err;
    reader->parse(&str[0], &str[str.size() - 1], &object, &err);
    return object;
}

int main() {
    Json::Value object = Deserialize(Serialize());

    std::cout << "================" << std::endl;
    std::cout << object["age"] << std::endl;
    std::cout << object["gender"] << std::endl;
    std::cout << object["skills"] << std::endl;
    return 0;
}
