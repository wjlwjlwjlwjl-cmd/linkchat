#include <jsoncpp/json/json.h>

#include "adapter/json/json_codec.hpp"

namespace ENTITY{
    /// @brief Json 基类
    class Base{
    public:
        using ptr = std::shared_ptr<Base>;
        Base(const std::string& name)
            : _name(name)
        {}

        template <typename T>
        Base& set(const std::string& key, const T& val){
            _data[key] = val;
            return *this;
        }

        template <typename T>
        Base& append(const std::string& key, const T& val){
            _data[key].append(val);
        }

        virtual Json::Value get(){
            return _data;
        }

        std::string key(){
            return _name;
        }

        virtual std::string to_string(){
            return JsonCodec::serialize(_data);
        }

        virtual void clear(){
            _data.clear();
            _name = "";
        }
    private:
        std::string _name;
        Json::Value _data;
    };

    class Object: public Base{
    public:
        using ptr = std::shared_ptr<Object>;
        Object(const std::string& name)
            : _name(name)
        {}

        Object& add(const std::string& name, Base::ptr ptr){
            _datas.insert(std::make_pair(name, ptr));
            return *this;
        }

        // 在 get 中，组装出json返回体
        virtual Json::Value get(){
            Json::Value root;
            for(auto& [name, data]: _datas){
                root[name] = data->get();
            }
            return root;
        }

        virtual Base::ptr children(const std::string& name){
            if(!_datas.count(name)){
                throw std::runtime_error("children not found");
            }
            return _datas[name];
        }

        virtual void clear(){
            _datas.clear();
            _name = "";
        }
    private:
        std::string _name;
        std::unordered_map<std::string, Base::ptr> _datas;
    };
}

/*

POST /user/_doc
{
  "settings": {
    "analysis": {
      "analyzer": {
        "ikmax": {
          "tokenizer": "ik_max_word"
        }
      }
    }
  }, 
  "mappings": {
    "dynamic": false,
    "properties": {
      "user_id": {
        "type": "keyword"
      },
      "message": {
        "type": "text",
        "analyzer": "ikmax"
      }
    }
  }
}

*/
namespace ES_ENTITY{
    /*
        "settings": {
            "analysis": {
                "analyzer": {
                    "ikmax": {
                        "tokenizer": "ik_max_word"
                    }
                }
            }
        }, 
    */
    class Settings: public ENTITY::Object{
    public:
        Settings& tokenizer(const std::string& tokennizer = "ik_max_word"){
            Base ikmax("ikmax");
            ikmax.set("tokenizer", "ik_max_word");
            Base 
        }
    };

    class Mappings{

    };
}