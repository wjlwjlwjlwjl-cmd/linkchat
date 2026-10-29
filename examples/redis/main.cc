#include <sw/redis++/redis.h>
#include <iostream>
#include <unordered_set>

std::shared_ptr<sw::redis::Redis> InitRedis() {
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
void TestString(std::shared_ptr<sw::redis::Redis>& redis_client) {
    // string 操作
    redis_client->set("key1", "val1");
    std::vector<std::pair<std::string, std::string>> strs;
    strs.push_back({"key2", "val2"});
    strs.push_back({"key3", "val3"});
    redis_client->mset(strs.begin(), strs.end());

    std::vector<std::string> keys = {"key1", "key2", "key4"};
    std::vector<std::optional<std::string>> values;
    redis_client->mget(keys.begin(), keys.end(), std::back_inserter(values));

    for (auto value : values) {
        std::cout << (value.has_value() ? value.value() : "") << std::endl;
    }
}

// list 操作
void TestList(std::shared_ptr<sw::redis::Redis>& redis_client) {
    redis_client->lpush("list1", "item1");
    std::vector<std::string> items = {
        "item2", "item3"
    };
    redis_client->lpush("list1", items.begin(), items.end());

    auto ret = redis_client->lpop("list1");
    if (ret.has_value())
        std::cout << ret.value() << std::endl;

    std::vector<std::string> results;
    redis_client->lrange("list1", 0, -1, std::back_inserter(results));
    for (auto item : results) {
        std::cout << item << std::endl;
    }
}

//hash 操作
void TestHash(std::shared_ptr<sw::redis::Redis>& redis_client) {
    redis_client->hset("hash1", {"key1", "val1"});
    std::vector<std::pair<std::string, std::string>> pairs = {
        {"key2", "val2"}, {"key3", "val3"}
    };
    redis_client->hmset("hash1", pairs.begin(), pairs.end());

    std::unordered_map<std::string, std::string> hash;
    redis_client->hgetall("hash1", std::inserter(hash, hash.begin()));

    for (auto [key, value] : hash) {
        std::cout << key << ": " << value << std::endl;
    }
}

//set 操作
void TestSet(std::shared_ptr<sw::redis::Redis>& redis_client) {
    redis_client->sadd("set1", "item1");
    std::unordered_set<std::string> items = {"item2", "item3"};
    redis_client->sadd("set1", items.begin(), items.end());

    items.clear();
    redis_client->smembers("set1", std::inserter(items, items.begin()));

    for (auto item : items) {
        std::cout << item << std::endl;
    }
}

int main() {
    auto redis_client = InitRedis();
    redis_client->flushall();

    //TestString(redis_client);
    //TestList(redis_client);
    //TestHash(redis_client);
    TestSet(redis_client);

    return 0;
}
