// brpc 异步调用需要使用 NewCallback 来完成对结果的获取，这里进行封装，这里只需要传来一个 void(*)();


#pragma once
#include <google/protobuf/stubs/callback.h>
#include <brpc/channel.h>
#include <functional>
#include <memory>

class CallbackFactory {
public:
    using callback_t = std::function<void()>;
    inline static ::google::protobuf::Closure* CreateCallback(callback_t&& callback);
private:
    struct Object {
        using ptr = std::shared_ptr<Object>;
        callback_t callback;
    };
    static void AsyncCallback(Object::ptr obj) {
        obj->callback();
    }
};

::google::protobuf::Closure* CallbackFactory::CreateCallback(callback_t&& callback) {
    Object::ptr obj = std::make_shared<Object>();
    obj->callback = std::move(callback);
    return brpc::NewCallback(AsyncCallback, obj);
}
