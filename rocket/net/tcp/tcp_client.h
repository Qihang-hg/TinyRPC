//
// Created by qihang on 23-9-6.
//

#ifndef TINYRPC_TCP_CLIENT_H
#define TINYRPC_TCP_CLIENT_H

#include "net_addr.h"
#include "../eventloop.h"
#include "tcp_connection.h"
#include "abstract_protocol.h"

namespace rocket{

class TcpClient {
public:
    TcpClient(NetAddr::s_ptr peer_addr);
    ~TcpClient();

    //异步的进行 connect
    //如果connect成功， done会被执行
    void connect(std::function<void()> done);

    //异步发送message
    //如果发送message,会调用done,done的入参是message对象
    void writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    //异步读取message
    //如果读取message成功,会调用done,done的入参是message对象
    void readMessage(const std::string &req_id, std::function<void(AbstractProtocol::s_ptr)> done);

private:
    NetAddr::s_ptr m_peer_addr; //保存的对端地址
    EventLoop* m_event_loop{NULL};  //当前客户所在的eventloop

    int m_fd{-1};
    FdEvent* m_fd_event{NULL};

    TcpConnection::s_ptr m_connection;

};

}




#endif //TINYRPC_TCP_CLIENT_H
