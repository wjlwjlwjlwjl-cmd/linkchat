// brpc 异步调用需要使用 NewCallback 来完成对结果的获取，这里进行封装，这里只需要传来一个 void(*)();


#pragma once
#include <google/protobuf/stubs/callback.h>
#include <brpc/channel.h>
#include <functional>
#include <memory>

class CallbackFactory {
public:
    using Callback = std::function<void()>;
    inline static ::google::protobuf::Closure* CreateCallback(Callback&& callback);
private:
    struct Object {
        using Ptr = std::shared_ptr<Object>;
        Callback callback;
    };
    static void AsyncCallback(Object::Ptr obj) {
        obj->callback();
    }
};

::google::protobuf::Closure* CallbackFactory::CreateCallback(Callback&& callback) {
    Object::Ptr obj = std::make_shared<Object>();
    obj->callback = std::move(callback);
    return brpc::NewCallback(AsyncCallback, obj);
}
