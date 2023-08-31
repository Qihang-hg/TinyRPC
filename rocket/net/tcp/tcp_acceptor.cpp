//
// Created by qihang on 23-8-27.
//

#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include "tcp_acceptor.h"
#include "../../common/log.h"
#include "net_addr.h"


namespace rocket{

TcpAcceptor::TcpAcceptor(NetAddr::s_ptr local_addr):m_local_addr(local_addr){
    if(!local_addr->checkValid()){
        ERRORLOG("invalid local addr %s", local_addr->toString().c_str());
        exit(0);
    }

/*
 * TCP 用SOCK_STREAM
 *     SOCK_STREAM     Provides sequenced, reliable, two-way, connection-based
 *     byte  streams.  An out-of-band data transmission mecha‐
 *     nism may be supported.
 */
    m_family = m_local_addr->getFamily();
    m_listenfd = socket(m_family, SOCK_STREAM,0);
    if(m_listenfd < 0){
        ERRORLOG("invalid listenfd %d", m_listenfd);
        exit(0);
    }
    //设置监听套接字为非阻塞
    //希望所有操作可以异步完成。当发送过来的字节序列不完整时候 希望不阻塞

    //SOL_SOCKET .level定义了哪个选项将被使用。通常情况下是SOL_SOCKET，意思是正在使用的socket选项。
    //一般来说，一个端口释放后会等待两分钟之后才能再被使用，SO_REUSEADDR是让端口释放后立即就可以被再次使用。
    int val = 1;
    if(setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){
        ERRORLOG("set SO_REUSEADDR error, errno = %d, error = %s", errno, strerror(errno));
    }

    socklen_t len = m_local_addr->getSockLen();
    if(bind(m_listenfd, m_local_addr->getSockAddr(),len) != 0){
        ERRORLOG("bind error, errno = %d, error = %s", errno, strerror(errno));
        exit(0);
    }

    if(listen(m_listenfd, 1000) != 0){
        ERRORLOG("listen error, errno = %d, error = %s", errno, strerror(errno));
        exit(0);
    }

}

TcpAcceptor::~TcpAcceptor(){}

std::pair<int, NetAddr::s_ptr> TcpAcceptor::accept() {
    if(m_family == AF_INET){
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_addr_len = sizeof(client_addr);

        int client_fd = ::accept(m_listenfd, reinterpret_cast<sockaddr*>(&client_addr), &client_addr_len);
        if(client_fd < 0){
            ERRORLOG("accept error, errno = %d, error = %s", errno, strerror(errno));
            exit(0);
        }
        IPNetAddr::s_ptr peer_addr = std::make_shared<IPNetAddr>(client_addr);
        INFOLOG("A client have been accepted successfully, peer addr [%s]",peer_addr->toString().c_str());
        return std::make_pair(client_fd,peer_addr);

    }else{
        //其他协议族 未实现
        return std::make_pair(-1, nullptr);
    }
}

int TcpAcceptor::getListenFd() {
    return m_listenfd;
}


}