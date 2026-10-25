#pragma once

#include <jsoncpp/json/json.h>
#include "../logging/logger.hpp"

class JsonCodec {
public:
    /**
     *
     * @param value 待序列化对象
     * @return 序列化字符串
     * @throw 序列化错误抛异常
     */
    static std::string serialize(const Json::Value& value) {
        auto builder = Json::StreamWriterBuilder();
        builder["indentation"] = "  ";           // 缩进改为 2 个空格（默认是制表符 \t）
        builder["commentStyle"] = "None";        // 不输出注释，避免干扰
        builder["enableYAMLCompatibility"] = true; // 在冒号后加空格，例如 "key": "value"

        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        std::stringstream ss;
        try {
            writer->write(value, &ss);
        }
        catch (const std::exception& e) {
            ERR("Json serialization fail: {}", e.what());
            throw;
        }
        return ss.str();
    }

    /**
     *
     * @param str 待反序列化字符串
     * @return 反序列化对象
     * @throw 反序列化失败抛异常
     */
    static Json::Value deserialize(const std::string& str) {
        auto builder = Json::CharReaderBuilder();
        auto reader = builder.newCharReader();
        std::unique_ptr<Json::CharReader> readerPtr(builder.newCharReader());
        Json::Value obj;
        std::string err;
        bool ret = reader->parse(&str[0], &str[str.size()], &obj, &err);
        if (!ret) {
            ERR("Json deserialize fail: {}", err);
            throw;
        }
        return obj;
    }
};