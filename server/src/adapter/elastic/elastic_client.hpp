#pragma once

#include <elasticlient/client.h>
#include <cpr/response.h>
#include <string>
#include <jsoncpp/json/json.h>

#include "adapter/json/json_codec.hpp"
#include "adapter/logging/logger.hpp"

class ElasticClient{
public:
    using Value = Json::Value;
    using PropType = std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>>;
    using DataType = std::vector<std::pair<std::string, std::string>>;
    ElasticClient() = delete;

    ElasticClient(const std::string& index_name, const std::string& doc_type = "_doc", const std::string id = "", 
        const std::vector<std::string>& urls = {"http://elastic:123456@elasticsearch-service:9200/"})
        : _client(std::make_shared<elasticlient::Client>(urls))
        , _index_name(index_name)
        , _doc_type(doc_type)
        , _id(id)
    {}

    bool CreateIndex(PropType& properties, const std::string& type = "custom", 
        const std::string& tokenizer = "ik_max_word", bool dynamic = false
    ){
        Value root;
        Value ikmax;
        ikmax["type"] = type;
        ikmax["tokenizer"] = tokenizer;
        Value analyzer;
        analyzer["ikmax"] = ikmax;
        Value analysis;
        analysis["analyzer"] = analyzer;
        Value settings;
        settings["analysis"] = analysis;
        root["settings"] = settings;

        Value mappings;
        mappings["dynamic"] = dynamic;
        Value prop;
        for(auto& [name, pair]: properties){
            Value item;
            for(auto& [key, val]: pair){
                item[key] = val;
            }
            prop[name] = item;
        }
        mappings["properties"] = prop;
        root["mappings"] = mappings;

        std::string body = JsonCodec::serialize(root);
        auto resp = _client->index(_index_name, _doc_type, _id, body);
        if(resp.status_code < 200 || resp.status_code >= 300){
            ERR("Create es index fail, {}", resp.text);
            return false;
        }
        INF("Create es index success, {}", resp.text);
        return true;
    }

    bool AddData(DataType& data){
        Value root;
        for(auto [key, val]: data){
            root[key] = val;
        }
        std::string body = JsonCodec::serialize(root);
        auto resp = _client->index(_index_name, _doc_type, _id, body);
        if(resp.status_code < 200 || resp.status_code >= 300){
            ERR("Add data to {} fail, {}", _index_name, resp.text);
            return false;
        }
        INF("Add data to {} success", _index_name);
        return true;
    }

    Value SelectTerm(const std::string& term, const std::string& val){
        Value root;
        root["query"]["term"][term] = val;
        std::string body = JsonCodec::serialize(root);
        auto resp = _client->search(_index_name, _doc_type, body);
        if(resp.status_code < 200 || resp.status_code >= 300){
            ERR("es precise search fail: {}", resp.text);
            return Value();
        }
        INF("es precise search success");
        return JsonCodec::deserialize(resp.text);
    }

    Value SelectMatch(const std::string& match, const std::string& val){
        Value root;
        root["query"]["match"][match] = val;
        std::string body = JsonCodec::serialize(root);
        auto resp = _client->search(_index_name, _doc_type, body);
        if(resp.status_code < 200 || resp.status_code >= 300){
            ERR("es match search fail: {}", resp.text);
            return Value();
        }
        INF("es match search success");
        return JsonCodec::deserialize(resp.text);
    }

    std::string Id(const Value& raw, int idx){
        Value hits = raw["hits"]["hits"];
        if(!hits.isArray() || hits.size() <= idx){
            return "";
        }

        const Json::Value& hit = hits[idx];
        if (!hit.isObject() || !hit.isMember("_id") || !hit["_id"].isString()) {
            return "";
        }
        return hit["_id"].asString();
    }

    void Remove(const std::string& id){
        auto resp = _client->remove(_index_name, _doc_type, id);
        if(resp.status_code < 200 || resp.status_code >= 300){
            ERR("es delete fail: {}", resp.text);
            return;
        }
        INF("es delete success");
    }
private:
    std::string _index_name;
    std::string _doc_type;
    std::string _id;
    std::shared_ptr<elasticlient::Client> _client;
};