//
// Created by qihang on 23-8-27.
//

#ifndef TINYRPC_TCP_ACCEPTOR_H
#define TINYRPC_TCP_ACCEPTOR_H

#include <memory>
#include "net_addr.h"

namespace rocket{

class TcpAcceptor {
public:
    typedef std::shared_ptr<TcpAcceptor> s_ptr;

    TcpAcceptor(NetAddr::s_ptr local_addr);
    ~TcpAcceptor();

    int accept();

    int getListenFd();

private:
    //服务端监听的地址addr -> ip:port
    //将结构体封装为类
    NetAddr::s_ptr m_local_addr;//封装了地址等信息

    int m_family{-1};
    int m_listenfd{-1};//listenfd 监听套接字
};

}



#endif //TINYRPC_TCP_ACCEPTOR_H
