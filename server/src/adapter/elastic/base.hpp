#include <jsoncpp/json/json.h>

#include "json/json_codec.hpp"

/// @brief Json 基类
class Base{
public:
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

    Json::Value val(){
        return _data;
    }

    std::string to_string(){
        //return 
    }
private:
    std::string _name;
    Json::Value _data;
};