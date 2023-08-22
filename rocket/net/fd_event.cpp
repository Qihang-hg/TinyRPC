//
// Created by qihang on 23-8-21.
//

#include <string.h>
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

std::function<void()> FdEvent::handler(TriggerEvent event_type){
    if(event_type == TriggerEvent::IN_EVENT){
        return m_read_callback;
    }else{
        return m_write_callback;
    }
}

void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback){
    if(event_type == TriggerEvent::IN_EVENT){
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;
    }else{
        m_listen_events.events |= EPOLLOUT;
        m_read_callback = callback;
    }
    m_listen_events.data.ptr = this;
}

}