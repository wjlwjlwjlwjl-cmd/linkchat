#include <etcd/Client.hpp>
#include <etcd/KeepAlive.hpp>

void Put(const std::string& host_ip,
    const std::string& key,
    const std::string& val,
    std::shared_ptr<::etcd::KeepAlive>& keepalive
){
    ::etcd::Client client(host_ip);
    auto lease = client.leasegrant(5).get(); //获取租约并拿到租约id，类似Redis中的ttl
    if(!lease.is_ok()){
        std::cout << "create lease fail" << lease.error_message() << std::endl;
        return;
    }
    long lease_id = lease.value().lease();
    auto lease_ret = client.put(key, val, lease_id).get();
    if(lease_ret.error_code() != 0){
        std::cout << "kv put fail: " << lease_ret.error_message() << std::endl;
        return;
    }
    std::cout << "kv put successfuly" << std::endl;
    auto handler = [&host_ip, &key, &val, &keepalive, &lease_id](std::exception_ptr e){
        try{
            Put(host_ip, key, val, keepalive);
        }
        catch(std::exception_ptr e){
            std::cout << "keeyalive exception handler failed to reput" << std::endl;
        }
    };
    keepalive.reset(new ::etcd::KeepAlive(host_ip, handler, 5, lease_id)); //设置租约保活，设置一个线程不断向etcd服务器“续租”
    std::cout << "keepalive set successful" << std::endl;
}

int main(){
    std::string host_ip = "http://etcd-service:2379"; //同一个docker网络中，可以使用服务名解析出容器ip
    std::shared_ptr<::etcd::KeepAlive> keepalive;

    std::string key = "key1/val1";
    std::string val = "val1";

    Put(host_ip, key, val, keepalive);

    getchar();

    return 0;
}