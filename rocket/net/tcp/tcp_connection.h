//
// Created by qihang on 23-8-31.
//

#ifndef TINYRPC_TCP_CONNECTION_H
#define TINYRPC_TCP_CONNECTION_H

#include <queue>
#include <memory>
#include "net_addr.h"
#include "tcp_buffer.h"
#include "../io_thread.h"
#include "abstract_protocol.h"
#include "../abstract_coder.h"


namespace rocket{

//连接状态
enum TcpState{
    NotConnected = 1,
    Connected = 2,
    HalfClosing = 3,
    Closed = 4
};

enum TcpConnectionType {
    TcpConnectionByServer = 1,
    TcpConnectionByClient = 2, //客户端使用 代表与服务端的连接
};

class TcpConnection {
public:
    typedef std::shared_ptr<TcpConnection> s_ptr;
public:
    TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr,TcpConnectionType type = TcpConnectionByServer);
    ~TcpConnection();

    void onRead();

    void excute();

    void onWrite();

    void setState(const TcpState state);
    TcpState getState();

    void clear();

    void shutdown();//服务器主动关闭连接.对于恶意连接，连接后不读写 占用资源

    void setConnectionType(TcpConnectionType type);

    void listenWrite(); //启动监听可写事件
    void listenRead(); //启动监听可读事件

    void pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done);

    void pushReadMessage(const std::string &req_id, std::function<void(AbstractProtocol::s_ptr)> done);

private:
    EventLoop* m_event_loop{NULL};    //当前connection属于的那个线程

    NetAddr::s_ptr m_local_addr;
    NetAddr::s_ptr m_peer_addr;//对端地址

    TcpBuffer::s_ptr m_in_buffer;   //receive buffer
    TcpBuffer::s_ptr m_out_buffer;


    FdEvent* m_fd_event{NULL};  //connection的套接字 本质也属于一个事件

    AbstractCoder* m_coder{NULL};

    TcpState m_state;
    int m_fd{-1};//客户套接字

    TcpConnectionType m_connection_type {TcpConnectionByServer};

    //std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>
    //里面是pair对象，绑定智能指针 和 要执行的func。也就是要写的 messages
    std::vector<std::pair<AbstractProtocol::s_ptr, std::function<void(AbstractProtocol::s_ptr)>>> m_write_dones;  //写事件的回调函数

    //key 为req-id
    std::map<std::string, std::function<void(AbstractProtocol::s_ptr)>> m_read_dones;  //读事件的回调函数


};


}



#endif //TINYRPC_TCP_CONNECTION_H
