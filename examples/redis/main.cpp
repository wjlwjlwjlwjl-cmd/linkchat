#include <sw/redis++/redis.h>
#include <iostream>
#include <unordered_set>

std::shared_ptr<sw::redis::Redis> init(){
    sw::redis::ConnectionOptions options;
    options.host = "redis-service";
    options.port = 6379;
    options.keep_alive = true;
    options.password = "123456";

    sw::redis::ConnectionPoolOptions pool_options;
    pool_options.size = 3;

    return std::make_shared<sw::redis::Redis>(options, pool_options);
}

// string 操作
void test_string(std::shared_ptr<sw::redis::Redis>& redis_client){
    // string 操作
    redis_client->set("key1", "val1");
    std::vector<std::pair<std::string, std::string>> strs;
    strs.push_back({"key2", "val2"});
    strs.push_back({"key3", "val3"});
    redis_client->mset(strs.begin(), strs.end());

    std::vector<std::string> keys = {"key1", "key2", "key4"};
    std::vector<std::optional<std::string>> v;
    redis_client->mget(keys.begin(), keys.end(), std::back_inserter(v));
    
    for(auto e: v){
        std::cout << (e.has_value()? e.value() : "") << std::endl;
    }
}

// list 操作
void test_list(std::shared_ptr<sw::redis::Redis>& redis_client){
    redis_client->lpush("list1", "item1");
    std::vector<std::string> items = {
        "item2", "item3"
    };
    redis_client->lpush("list1", items.begin(), items.end());

    auto ret = redis_client->lpop("list1");
    if(ret.has_value())
        std::cout << ret.value() << std::endl;

    std::vector<std::string> rets;
    redis_client->lrange("list1", 0, -1, std::back_inserter(rets));
    for(auto e: rets){
        std::cout << e << std::endl;
    }
}

//hash 操作
void test_hash(std::shared_ptr<sw::redis::Redis>& redis_client){
    redis_client->hset("hash1", {"key1", "val1"});
    std::vector<std::pair<std::string, std::string>> pairs = {
        {"key2", "val2"}, {"key3", "val3"}
    };
    redis_client->hmset("hash1", pairs.begin(), pairs.end());

    std::unordered_map<std::string, std::string> hash;
    redis_client->hgetall("hash1", std::inserter(hash, hash.begin()));

    for(auto [i, j]: hash){
        std::cout << i << ": " << j << std::endl;
    }
}

//set 操作
void test_set(std::shared_ptr<sw::redis::Redis>& redis_client){
    redis_client->sadd("set1", "item1");
    std::unordered_set<std::string> items = {"item2", "item3"};
    redis_client->sadd("set1", items.begin(), items.end());

    items.clear();
    redis_client->smembers("set1", std::inserter(items, items.begin()));

    for(auto e: items){
        std::cout << e << std::endl;
    }
}

int main(){
    auto redis_client = init();
    redis_client->flushall();

    //test_string(redis_client);
    //test_list(redis_client);
    //test_hash(redis_client);
    test_set(redis_client);

    return 0;
}