//amqp 操作封装，声明交换机、队列、绑定关系等；发布，订阅；启动网络时间循环


#pragma once

#include <string>
#include <amqpcpp/libev.h>
#include <amqpcpp.h>

class MQClient{
public:
    MQClient(const std::string& broker_url);
private:
    struct ev_loop* _ev_loop; //网络事件循环
    AMQP::TcpConnection _tcp_connection;
    AMQP::TcpChannel _tcp_channel;
    AMQP::LibEvHandler _ev_handler;
};

MQClient::MQClient(const std::string& broker_url){
    _ev_loop = EV_DEFAULT;
    _ev_handler = AMQP::LibEvHandler(_ev_loop);
    
}