#include <jsoncpp/json/json.h>

#include "adapter/json/json_codec.hpp"

namespace ES_ENTITY{
    /// @brief Json 基类
    class Base{
    public:
        using ptr = std::shared_ptr<Base>;
        Base(const std::string& name)
            : _name(name)
        {}

        template <typename T>
        void set(const std::string& key, const T& val){
            _data[key] = val;
        }

        template <typename T>
        void append(const std::string& key, const T& val){
            _data[key].append(val);
        }

        Json::Value get(){
            return _data;
        }

        std::string key(){
            return _name;
        }

        std::string to_string(){
            return JsonCodec::serialize(_data);
        }
    private:
        std::string _name;
        Json::Value _data;
    };

    class Object{
    public:
        using ptr = std::shared_ptr<Object>;
        Object(const std::string& name)
            : _name(name)
        {}

        void add(const std::string& name, Base::ptr ptr){
            _datas.insert(std::make_pair(name, ptr));
        }

        Json::Value get(){
            Json::Value root;
            for(auto& [name, data]: _datas){
                root[name] = data->get();
            }
            return root;
        }

        Base::ptr children(const std::string& name){
            if(!_datas.count(name)){
                throw std::runtime_error("children not found");
            }
            return _datas[name];
        }
    private:
        std::string _name;
        std::unordered_map<std::string, Base::ptr> _datas;
    };
}