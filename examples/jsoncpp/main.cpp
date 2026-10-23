#include <iostream>
#include <jsoncpp/json/json.h>

std::string serialize() {
    Json::Value obj;
    obj["name"] = "John";
    obj["age"] = 20;
    obj["gender"] = "Male";
    obj["skills"].append("C++");
    obj["skills"].append("Java");
    obj["skills"].append("Python");

    auto builder = Json::StreamWriterBuilder();
    builder["indentation"] = "  ";           // 缩进改为 2 个空格（默认是制表符 \t）
    builder["commentStyle"] = "None";        // 不输出注释，避免干扰
    builder["enableYAMLCompatibility"] = true; // 在冒号后加空格，例如 "key": "value"

    auto writer = builder.newStreamWriter();
    std::stringstream ss;
    try {
        writer->write(obj, &ss);
    }
    catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    std::cout << ss.str() << std::endl;
    return ss.str();
}

Json::Value unserialize(const std::string& str) {
    auto builder = Json::CharReaderBuilder();
    auto reader = builder.newCharReader();
    Json::Value obj;
    std::string err;
    reader->parse(&str[0], &str[str.size() - 1], &obj, &err);
    return obj;
}

int main() {
    Json::Value obj = unserialize(serialize());

    std::cout << "================" << std::endl;
    std::cout << obj["age"] << std::endl;
    std::cout << obj["gender"] << std::endl;
    std::cout << obj["skills"] << std::endl;
    return 0;
}