//
// Created by qihang on 23-8-31.
//

#include "fd_event_group.h"

namespace rocket{

static FdEventGroup* g_fd_event_group = NULL;

FdEventGroup::FdEventGroup(int size):m_size(size){
    for(int i = 0; i<m_size; ++i){
        m_fd_group.push_back(new FdEvent(i));
    }
}
FdEventGroup::~FdEventGroup(){
    for(size_t i = 0; i<m_size; ++i){
        if(m_fd_group[i] != NULL){
            delete m_fd_group[i];
            m_fd_group[i] = NULL;
        }
    }
}

FdEventGroup* FdEventGroup::GetFdEventGroup(){
    if(g_fd_event_group != NULL){
        return g_fd_event_group;
    }
    g_fd_event_group = new FdEventGroup(128);
    return g_fd_event_group;
}

FdEvent* FdEventGroup::getFdEvent(int fd){
    ScopeMutex<Mutex> lock(m_mutex);
    if(fd < m_fd_group.size()){
        m_fd_group[fd];
    }
    //扩容
    int new_size = fd * 1.5;
    for(int i = m_fd_group.size(); i<new_size; ++i){
        m_fd_group.push_back(new FdEvent(i));
    }
    return m_fd_group[fd];
}


}