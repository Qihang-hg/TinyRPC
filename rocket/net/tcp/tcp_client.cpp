//
// Created by qihang on 23-9-6.
//

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include "tcp_client.h"
#include "../../common/log.h"
#include "../eventloop.h"
#include "../fd_event_group.h"

namespace rocket{

TcpClient::TcpClient(NetAddr::s_ptr peer_addr):m_peer_addr(peer_addr) {
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
    if(m_fd < 0){
        ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
        return;
    }

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();
    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr);
    m_connection->setConnectionType(TcpConnectionByClient);
}
TcpClient::~TcpClient(){
    DEBUGLOG("TcpClient::~TcpClient()");
    if(m_fd > 0){
        close(m_fd);
    }
}

//异步的进行 connect
//如果connect成功， done会被执行
void TcpClient::connect(std::function<void()> done){
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if(rt == 0){
        DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
        if(done){
            done();
        }
    }else if( rt == -1){
        if(errno == EINPROGRESS){
            //epoll listen writable event ,then judge errno
            //epoll监听可写事件，监听到后执行可写事件的回调函数，回调函数中执行done
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done](){
                int error = 0;
                socklen_t  error_len = sizeof(error);
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR,&error, &error_len);
                if(error == 0){//连接建立成功
                    DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                    if(done){
                        done();
                    }
                }else{
                    ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));

                }
                m_fd_event->cancel(FdEvent::OUT_EVENT);
                m_event_loop->addEpollEvent(m_fd_event);
                m_event_loop->deleteEpollEvent(m_fd_event);
            });
            m_event_loop->addEpollEvent(m_fd_event);
            if(!m_event_loop->isLooping()){
                m_event_loop->loop();
            }

        }else{
            ERRORLOG("connect error, errno = %d, error = %s", errno, strerror(errno));
        }
    }
}

//异步发送message
//如果发送message,会调用done,done的入参是message对象
void TcpClient::writeMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done){

}

//异步读取message
//如果读取message成功,会调用done,done的入参是message对象
void TcpClient::readMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done){}

}