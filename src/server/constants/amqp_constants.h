#pragma once

#include <string>
#include <amqpcpp.h>

#include "logging/logger.h"

//交换机类型
namespace exchange_type {
    inline constexpr char kFanout[] = "fanout";
    inline constexpr char kDirect[] = "direct";
    inline constexpr char kTopic[] = "topic";
    inline constexpr char kHeaders[] = "headers";

    inline AMQP::ExchangeType GetExchangeType(const std::string& type) {
        if (type == kFanout) {
            return AMQP::ExchangeType::fanout;
        }
        else if (type == kDirect) {
            return AMQP::ExchangeType::direct;
        }
        else if (type == kTopic) {
            return AMQP::ExchangeType::topic;
        }
        else if (type == kHeaders) {
            return AMQP::ExchangeType::headers;
        }
        else {
            LOG_ERROR("unsupported exchange type");
            abort();
        }
    }
}

//死信队列相关
namespace dead_letter {
    inline constexpr char kDlxPrefix[] = "dlx_";
    inline constexpr char kDlqPrefix[] = "dlq_";
    inline constexpr char kDlxBindingKey[] = "dlx_binding_";

    inline constexpr char kDlxOp[] = "x-dead-letter-exchange";
    inline constexpr char kDlqOp[] = "x-dead-letter-routing-key";
    inline constexpr char kMsgTtl[] = "x-message-ttl";
}
