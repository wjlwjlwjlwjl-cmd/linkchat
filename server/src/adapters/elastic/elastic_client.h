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

    ElasticClient(const std::string& index_name, const std::string& doc_type = "_doc", const std::string id = "",
        const std::vector<std::string>& urls = {"http://elastic:123456@elasticsearch-service:9200/"})
        : client_(std::make_shared<elasticlient::Client>(urls))
        , index_name_(index_name)
        , doc_type_(doc_type)
        , id_(id)
    {}

    bool CreateIndex(PropType& properties, const std::string& type = "custom",
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
        auto resp = client_->index(index_name_, doc_type_, id_, body);
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

    Value SelectTerm(const std::string& term, const std::string& val) {
        Value root;
        root["query"]["term"][term] = val;
        std::string body = JsonCodec::Serialize(root);
        auto resp = client_->search(index_name_, doc_type_, body);
        if (resp.status_code < 200 || resp.status_code >= 300) {
            LOG_ERROR("es precise search fail: {}", resp.text);
            return Value();
        }
        LOG_INFO("es precise search success");
        return JsonCodec::Deserialize(resp.text);
    }

    Value SelectMatch(const std::string& match, const std::string& val) {
        Value root;
        root["query"]["match"][match] = val;
        std::string body = JsonCodec::Serialize(root);
        auto resp = client_->search(index_name_, doc_type_, body);
        if (resp.status_code < 200 || resp.status_code >= 300) {
            LOG_ERROR("es match search fail: {}", resp.text);
            return Value();
        }
        LOG_INFO("es match search success");
        return JsonCodec::Deserialize(resp.text);
    }

    std::string Id(const Value& raw, int index) {
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

    void Remove(const std::string& id) {
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
