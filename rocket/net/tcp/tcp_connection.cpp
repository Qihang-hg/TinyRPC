//
// Created by qihang on 23-8-31.
//

#include <unistd.h>
#include "tcp_connection.h"
#include "../fd_event_group.h"
#include "../../common/log.h"
#include "../string_coder.h"

namespace rocket{

TcpConnection::TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, TcpConnectionType type/* = TcpConnectionByServer*/)
    :m_event_loop(event_loop), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd), m_connection_type(type){
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    //服务端才主动监听读事件 客户端在需要的时候监听读事件
    if(m_connection_type == TcpConnectionByServer){
        listenRead();
    }
    //将对客户的监听事件添加到线程的eventloop中去
//    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
//    m_event_loop->addEpollEvent(m_fd_event);

    m_coder = new StringCoder();
    DEBUGLOG("TcpConnection construct");



}

TcpConnection::~TcpConnection(){
    DEBUGLOG("~TcpConnection");
    if(m_coder){
        delete m_coder;
        m_coder = NULL;
    }
}

void TcpConnection::onRead(){
    //还未区分 客户端和服务端在读上面的 执行逻辑区别，且 m_read_dones 回调函数 也需要判断是否执行完(参考 write)


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
    // server端先简单处理：将RPC请求执行业务逻辑，获取RPC响应，再把RPC响应发送回去
    if(m_connection_type == TcpConnectionByServer){
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
        listenWrite();
        DEBUGLOG("onWrite 写事件添加成功");
    }else{
        //client：从 buffer decode得到message对象，判断是否与req_id(响应的id 是否与 发送的请求id)相等，则读成功 ,执行回调
        std::vector<AbstractProtocol::s_ptr> result;
        m_coder->decode(result, m_in_buffer);

        for(size_t i = 0; i<result.size(); ++i){
            std::string req_id = result[i]->getReqId();
            auto it = m_read_dones.find(req_id);
            if(it != m_read_dones.end()){
                it->second(result[i]);

            }

        }
    }



}

void TcpConnection::onWrite(){
    DEBUGLOG("excute onWrite")
    //将当前out_buffer里面的数据发送到client
    if(m_state != Connected){
        ERRORLOG("onWrite error, client has already disconected, adde[%s], clientfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    // 如果当前是在连接的客户端
    if(m_connection_type == TcpConnectionByClient){
        //1.将message encode得到字节流
        //2.将字节流写入到 out_buffer,然后全部发送
        std::vector<AbstractProtocol::s_ptr> messages;
        for(size_t i = 0; i<m_write_dones.size(); ++i){
            messages.push_back(m_write_dones[i].first);// 智能指针的get方法，获取托管的指针，为AbstractProtocol*
        }
        m_coder->encode(messages, m_out_buffer);//方大盘发送缓冲区
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
        m_event_loop->addEpollEvent(m_fd_event);
    }

    if(m_connection_type == TcpConnectionByClient){
        for(size_t i = 0; i<m_write_dones.size(); ++i){
            m_write_dones[i].second(m_write_dones[i].first); //发送后 依次执行回调函数
        }

        m_write_dones.clear();  //执行完回调函数后做清空
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
    m_event_loop->addEpollEvent(m_fd_event);
    m_event_loop->deleteEpollEvent(m_fd_event);

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

void TcpConnection::setConnectionType(TcpConnectionType type){
    m_connection_type = type;
}

void TcpConnection::listenWrite(){
    //监听写事件并添加到epoll
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_event_loop->addEpollEvent(m_fd_event);
}
void TcpConnection::listenRead(){
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done){
    m_write_dones.push_back(std::make_pair(message,done));
}

void TcpConnection::pushReadMessage(const std::string &req_id, std::function<void(AbstractProtocol::s_ptr)> done){
    m_read_dones.insert(std::make_pair(req_id, done));
}



}
