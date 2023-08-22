//
// Created by qihang on 23-8-21.
//

#ifndef TINYRPC_FD_EVENT_H
#define TINYRPC_FD_EVENT_H

#include <functional>
#include <sys/epoll.h>

namespace rocket{

class FdEvent {
public:
    enum TriggerEvent{
        IN_EVENT = EPOLLIN,
        OUT_EVENT = EPOLLOUT,
        ERROR_EVENT = EPOLLERR
    };
    FdEvent();
    FdEvent(int fd);
    virtual ~FdEvent();

    std::function<void()> handler(TriggerEvent event_type);

    void listen(TriggerEvent event_type, std::function<void()> callback);
    int getFd() const {
        return m_fd;
    }
    epoll_event getEpollEvent(){
        return m_listen_events;
    }

protected:
    int m_fd {-1};
    epoll_event m_listen_events;

    std::function<void()> m_read_callback{nullptr};
    std::function<void()> m_write_callback{nullptr};
};


}



#endif //TINYRPC_FD_EVENT_H
