#include <string>

//交换机类型
namespace EXCHANGE_TYPE {
    static const std::string FANOUT = "fanout";
    static const std::string DIRECT = "direct";
    static const std::string TOPIC = "topic";
    static const std::string HEADER = "header";
}

//死信队列相关
namespace DL {
    static const std::string DLX_PREFIX = "dlx_";
    static const std::string DLQ_PREFIX = "dlq_";
    static const std::string DLX_ROUTING_KEY = "dlx_binding_";
}