#pragma once

#include <elasticlient/client.h>
#include <cpr/response.h>
#include <string>
#include <jsoncpp/json/json.h>

#include "json/json_codec.h"
#include "logging/logger.h"

class ElasticClient {
public:
    using Value = Json::Value;

    /*
        "content": {
            "type": "text",
            "analyzer": "ikmax"
        }
    */
    using PropType = std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>;
    using DataType = std::vector<std::pair<std::string, std::string>>;
    ElasticClient() = delete;

    inline ElasticClient(const std::string& index_name, const std::string& doc_type = "_doc", const std::string id = "",
        const std::vector<std::string>& urls = {"http://elastic:123456@elasticsearch-service:9200/"})
        : client_(std::make_shared<elasticlient::Client>(urls))
        , index_name_(index_name)
        , doc_type_(doc_type)
        , id_(id)
    {}

    inline bool CreateIndex(PropType& properties, const std::string& type = "custom",
        const std::string& tokenizer = "ik_max_word", bool dynamic = false
    ) {
        Value root;
        Value ik_max;
        ik_max["type"] = type;
        ik_max["tokenizer"] = tokenizer;
        Value analyzer;
        analyzer["ikmax"] = ik_max;
        Value analysis;
        analysis["analyzer"] = analyzer;
        Value settings;
        settings["analysis"] = analysis;
        root["settings"] = settings;

        Value mappings;
        mappings["dynamic"] = dynamic;
        Value prop;
        for (auto& [name, pair] : properties) {
            Value item;
            for (auto& [key, val] : pair) {
                item[key] = val;
            }
            prop[name] = item;
        }
        mappings["properties"] = prop;
        root["mappings"] = mappings;

        std::string body = JsonCodec::Serialize(root);
        // 用 PUT /{index} 真正创建索引；索引已存在时 ES 返回 400，忽略即可，保证每次启动都可安全调用
        auto resp = client_->performRequest(elasticlient::Client::HTTPMethod::PUT, index_name_, body);
        if (resp.status_code == 400 && resp.text.find("resource_already_exists_exception") != std::string::npos) {
            LOG_INFO("es index {} already exists", index_name_);
            return true;
        }
        if (resp.status_code < 200 || resp.status_code >= 300) {
            LOG_ERROR("Create es index fail, {}", resp.text);
            return false;
        }
        LOG_INFO("Create es index success, {}", resp.text);
        return true;
    }

    bool AddData(DataType& data) {
        Value root;
        for (auto [key, val] : data) {
            root[key] = val;
        }
        std::string body = JsonCodec::Serialize(root);
        auto resp = client_->index(index_name_, doc_type_, id_, body);
        if (resp.status_code < 200 || resp.status_code >= 300) {
            LOG_ERROR("Add data to {} fail, {}", index_name_, resp.text);
            return false;
        }
        LOG_INFO("Add data to {} success", index_name_);
        return true;
    }

    /// @brief 精确查询 + 模糊匹配
    /// @param terms 精确查询字段 + 值
    /// @param matches 模糊匹配字段 + 值
    /// @return 结果 json 
    inline Value SelectTermAndMatch(
        std::vector<std::pair<std::string, std::string>>& terms, 
        std::vector<std::pair<std::string, std::string>>& matches
    ) {
        if(terms.empty() || matches.empty()){
            return Value();
        }
        Value root;
        Value must_array;

        for(auto& [key, val]: terms){
            if(key.empty() || val.empty()) continue;
            Value item;
            item["term"][key] = val;
            must_array.append(item);
        }
        for(auto& [key, val]: matches){
            if(key.empty() || val.empty()) continue;
            Value item;
            item["match"][key] = val;
            must_array.append(item);
        }
        root["query"]["bool"]["must"] = must_array;

        try{
            std::string body = JsonCodec::Serialize(root);
            auto resp = client_->search(index_name_, doc_type_, body);
            if (resp.status_code < 200 || resp.status_code >= 300) {
                LOG_ERROR("es precise search fail: {}", resp.text);
                return Value();
            }
            LOG_INFO("es search success");
            return JsonCodec::Deserialize(resp.text);
        }
        catch(const std::exception& e){
            LOG_ERROR("es search fail: {}", e.what());
            return Value();
        }
    }

    inline std::string Id(const Value& raw, int index) {
        Value hits = raw["hits"]["hits"];
        if (!hits.isArray() || hits.size() <= index) {
            return "";
        }

        const Json::Value& hit = hits[index];
        if (!hit.isObject() || !hit.isMember("_id") || !hit["_id"].isString()) {
            return "";
        }
        return hit["_id"].asString();
    }

    inline void Remove(const std::string& id) {
        if (id.empty()) {
            return;
        }
        auto resp = client_->remove(index_name_, doc_type_, id);
        if (resp.status_code < 200 || resp.status_code >= 300) {
            LOG_ERROR("es delete fail: {}", resp.text);
            return;
        }
        LOG_INFO("es delete success");
    }
private:
    std::string index_name_;
    std::string doc_type_;
    std::string id_;
    std::shared_ptr<elasticlient::Client> client_;
};
