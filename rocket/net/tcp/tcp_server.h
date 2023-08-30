//
// Created by qihang on 23-8-30.
//

#ifndef TINYRPC_TCP_SERVER_H
#define TINYRPC_TCP_SERVER_H

#include "tcp_acceptor.h"
#include "net_addr.h"
#include "../eventloop.h"
#include "../io_thread_group.h"

namespace rocket{

class TcpServer{
public:
    TcpServer(NetAddr::s_ptr local_addr);

    ~TcpServer();

    void start();


private:
    void init();//在主线程 主reactor中添加 监听套接字

    void onAccept();//当有新连接客户端后需要执行

private:
    TcpAcceptor::s_ptr m_acceptor;

    NetAddr::s_ptr m_local_addr;    //local listen address 本地监听地址 端口等

    FdEvent* m_listen_fd_event{NULL};

    EventLoop* m_main_event_loop{NULL}; //main Ractor, main thread eventloop

    IOThreadGroup* m_io_thread_group{NULL};// subReactor组

    int m_client_counts{0};

};

}



#endif //TINYRPC_TCP_SERVER_H
