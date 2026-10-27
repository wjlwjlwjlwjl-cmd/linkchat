#include <sw/redis++/redis.h>

namespace redis_factory{
    struct RedisConnectionInfo{
        std::string host = "redis-service";
        int port = 6379;
        std::string user = "default";
        std::string password = "123456";
        int db = 0;
        bool keep_alive = true;
        int pool_size = 3;
    };

    class RedisFactory{
    public:
        std::shared_ptr<sw::redis::Redis> create(const RedisConnectionInfo& info);
    };

    std::shared_ptr<sw::redis::Redis> RedisFactory::create(const RedisConnectionInfo& info){
        sw::redis::ConnectionOptions options;
        options.host = info.host;
        options.port = info.port;
        options.user = info.user;
        options.password = info.password;
        options.db = info.db;
        options.keep_alive = info.keep_alive;

        sw::redis::ConnectionPoolOptions pool_options;
        pool_options.size = info.pool_size;

        return std::make_shared<sw::redis::Redis>(options, pool_options);
    }
}
