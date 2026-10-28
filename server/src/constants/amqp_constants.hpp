#pragma once


#include <string>
#include <amqpcpp.h>

#include "adapter/logging/logger.hpp"

//交换机类型
namespace EXCHANGE_TYPE {
    static const std::string FANOUT = "fanout";
    static const std::string DIRECT = "direct";
    static const std::string TOPIC = "topic";
    static const std::string HEADERS = "headers";

    AMQP::ExchangeType getExchangeType(const std::string& type){
        if(type == FANOUT){
            return AMQP::ExchangeType::fanout;
        }
        else if(type == DIRECT){
            return AMQP::ExchangeType::direct;
        }
        else if(type == TOPIC){
            return AMQP::ExchangeType::topic;
        }
        else if(type == HEADERS){
            return AMQP::ExchangeType::headers;
        }
        else{
            ERR("unsupported exchange type");
            abort();
        }
    }
}

//死信队列相关
namespace DL {
    static const std::string DLX_PREFIX = "dlx_";
    static const std::string DLQ_PREFIX = "dlq_";
    static const std::string DLX_BINDING_KEY = "dlx_binding_";

    static const std::string DLX_OP = "x-dead-letter-exchange";
    static const std::string DLQ_OP = "x-dead-letter-routing-key";
    static const std::string MSG_TTL = "x-message-ttl";
}