//
// Created by qihang on 23-8-21.
//

#include <string.h>
#include <fcntl.h>
#include "fd_event.h"
#include "../common/log.h"

namespace rocket{



FdEvent::FdEvent(int fd):m_fd(fd) {
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::FdEvent() {
    memset(&m_listen_events, 0, sizeof(m_listen_events));
}

FdEvent::~FdEvent(){}

void FdEvent::setNonBlock() {
    int flag = fcntl(m_fd, F_GETFL, 0);
    if(flag & O_NONBLOCK){
        return;
    }

    fcntl(m_fd, F_SETFL, flag|O_NONBLOCK);
}

//取消监听事件
void FdEvent::cancel(TriggerEvent event_type){
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events.events &= (~EPOLLIN);// ~非 最好加括号，优先级容易有问题

    }else{
        m_listen_events.events &= (~EPOLLOUT);
    }
}



std::function<void()> FdEvent::handler(TriggerEvent event_type){
    if(event_type == IN_EVENT){
        return m_read_callback;
    }else if(event_type == OUT_EVENT){
        return m_write_callback;
    }else if(event_type == ERROR_EVENT){
        return m_error_callback;
    }
    return nullptr;
}

void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback) {
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;
    }else{
        m_listen_events.events |= EPOLLOUT;
        m_write_callback = callback;
    }
    m_listen_events.data.ptr = this;
}


}


