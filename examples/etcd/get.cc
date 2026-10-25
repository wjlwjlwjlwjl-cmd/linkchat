#include <etcd/Client.hpp>
#include <etcd/Watcher.hpp>
#include <functional>

void watcher_callback(::etcd::Response const& response){
    if(response.error_code() != 0){
        std::cout << "监听出错：" << response.error_message() << std::endl;
    }

    for(auto& ev: response.events()){
        if(ev.event_type() == ::etcd::Event::EventType::PUT){
            //新增键值对
            std::cout << "新增键值对：" << ev.kv().key() << " - " << ev.kv().as_string() << std::endl;
        }
        else if(ev.event_type() == ::etcd::Event::EventType::DELETE_){
            //键值对被删除
            std::cout << "删除键值对" << ev.kv().key() << " - " << ev.prev_kv().as_string() << std::endl;
        }
    }
}

void watch(const std::string& host_ip, 
    const std::string& key,
    std::shared_ptr<::etcd::Watcher>& watcher
){
    ::etcd::Client client(host_ip);
    auto resp = client.ls(key).get();
    if(resp.is_ok()){
        for(int i = 0; i < resp.keys().size(); i++){
            std::cout << resp.key(i) << " - " << resp.value(i).as_string() << std::endl;
        }
    }
    else{
        //获取键值对失败
        std::cout << "获取键值对失败" << std::endl;
    }
    
    auto cb = std::bind(watcher_callback, std::placeholders::_1);
    
    //etcd 中，一般使用 '/' 区分单词，recursive表示递归处理，例如 /service/chat 会监听到 /service/chat/node1 和 /service/chat/node2
    //watcher.reset(new ::etcd::Watcher(client, key, watcher_callback, true));
    watcher = std::make_shared<::etcd::Watcher>(client, key, cb, true);

    watcher->Wait([&](bool canceled){
        if(canceled){
            std::cout << "Etcd 主动断开监听" << std::endl;
            return;
        }
        watch(host_ip, key, watcher);
    });
}

int main(){
    std::string host_ip = "http://etcd-service:2379";
    std::string key = "key1";

    std::shared_ptr<::etcd::Watcher> watcher;

    watch(host_ip, key, watcher);

    getchar();

    return 0;
}