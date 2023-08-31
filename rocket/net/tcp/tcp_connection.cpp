//
// Created by qihang on 23-8-31.
//

#include <unistd.h>
#include "tcp_connection.h"
#include "../fd_event_group.h"
#include "../../common/log.h"

namespace rocket{

TcpConnection::TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr)
    :m_io_thread(io_thread), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd){
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    DEBUGLOG("TcpConnection construct");

    //将对客户的监听事件添加到线程的eventloop中去
    io_thread->getEventLoop()->addEpollEvent(m_fd_event);
}

TcpConnection::~TcpConnection(){
    DEBUGLOG("~TcpConnection");
}

void TcpConnection::onRead(){
    // 1.从 socket缓冲区，调用系统read读取字节到 in_buffer
    if(m_state != Connected){
        ERRORLOG("onRead error, client has already disconnected, addr[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd_event->getFd());
        return;
    }
    bool is_read_all = false;
    bool is_close = false;
    while(!is_read_all){
        if(m_in_buffer->writeAble() == 0){
            m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
        }
        int read_count = m_in_buffer->writeAble();
        int write_index = m_in_buffer->writeIndex();

        int  rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);
        if(rt > 0){
            m_in_buffer->moveWriteIndex(rt);
            if(rt == read_count){//socket缓冲区可能还没读完,读到为-1 或者0？
                continue;
            }else if(rt < read_count){
                is_read_all = true;//读完了
                break;
            }
        }else if(rt == 0){
            //读到可读事件，但是 读到的字节数为0 或者小于0.说明对端关了
            is_close = true;
            break;
        }else if(rt == -1 && errno == EAGAIN){
            is_read_all = true;
            break;
        }
    }
    //对端关闭
    if(is_close){
        //处理关闭连接
        clear();//clear the connection清除连接
        //有个问题 clear()delete了监听事件，但是后面excute又添加了。所以在这里return,不执行后面excute
        INFOLOG("peer closed, peer addr [%s], clientfd [%d]",m_peer_addr->toString().c_str(),m_fd);
        return;
    }
    //还没读完
    if(!is_read_all){
        ERRORLOG("not read all!");
    }

    //todo: 简单echo,后面补充RPC协议解析
    excute();
}

void TcpConnection::excute(){
    //将RPC请求执行业务逻辑，获取RPC响应，再把RPC响应发送回去
    std::vector<char> tmp;//将缓冲区内容读取到tmp
    int size = m_in_buffer->readAble();
    tmp.resize(size);
    m_in_buffer->readFromBuffer(tmp, size);//读到 读缓冲区

    //这里暂时原样作为响应发送回去，不对数据执行别的处理逻辑
    std::string msg;
    for(size_t i = 0; i<tmp.size(); ++i){
        msg += tmp[i];
    }

    INFOLOG("success get request[%s] from client addr[%s]", msg.c_str(), m_peer_addr->toString().c_str());

    // 从读缓冲区通过tmp 拷贝到了 写缓冲区
    m_out_buffer->writeToBuffer(msg.c_str(),msg.length());

    //执行完后,监听写事件并添加到epoll
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
    DEBUGLOG("onWrite 写事件添加成功");
}

void TcpConnection::onWrite(){
    DEBUGLOG("excute onWrite")
    //将当前out_buffer里面的数据发送到client
    if(m_state != Connected){
        ERRORLOG("onWrite error, client has already disconected, adde[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    //写缓冲区能读出来的就是要发送的
    bool is_write_all = false;//加个标志。避免一直监听写事件
    while(true){
        if(m_out_buffer->readAble() == 0){
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        //要发送的字节数量
        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();//读outbuffer的指针
        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);

        if(rt >= write_size){
            INFOLOG("all data has been sent to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        //socket缓冲区已满，不能再发送了.等下次fd可写,再发送即可
        if(rt == -1 && errno == EAGAIN){
            ERRORLOG("onWrite data error, errno=EAGAIN and rt = -1");
            break;
        }
    }

    //取消对写事件的监听，并更新到io线程的epoll中
    if(is_write_all){
        m_fd_event->cancel(FdEvent::OUT_EVENT);
        m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
    }
}

void TcpConnection::setState(const TcpState state) {
    m_state = state;
}

TcpState TcpConnection::getState() {
    return m_state;
}

void TcpConnection::clear() {
    //处理一些连接关闭后的清理动作
    if(m_state == Closed){
        return;
    }
    //取消对读写事件的监听。即使后面误加到 epoll,也无监听事件
    m_fd_event->cancel(FdEvent::IN_EVENT);
    m_fd_event->cancel(FdEvent::OUT_EVENT);
    m_io_thread->getEventLoop()->addEpollEvent(m_fd_event);
    m_io_thread->getEventLoop()->deleteEpollEvent(m_fd_event);

    m_state = Closed;
}

void TcpConnection::shutdown() {
    if((m_state == Closed) || (m_state == NotConnected)){
        return;
    }
    //处于半关闭
    m_state = HalfClosing;

    ::shutdown(m_fd,SHUT_RDWR);
    //需要四次挥手，调用shutdown系统调用关闭读写， 意味着服务器不会再对fd进行读写
    //发送 FIN报文，触发四次挥手第一个阶段，
    //当fd发生可读事件 且可读数据为0 即对端发送了FIN

}

}
