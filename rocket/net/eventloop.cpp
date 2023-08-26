//
// Created by qihang on 23-8-21.
//
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>
#include "eventloop.h"
#include "../common/log.h"
#include "../common/util.h"


#define ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd());\
    int op = EPOLL_CTL_ADD;\
    if(it != m_listen_fds.end()){ \
        op = EPOLL_CTL_MOD;\
    }\
    epoll_event tmp = event->getEpollEvent();   \
    INFOLOG("epoll_event.events = %d",(int)tmp.events);\
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);\
    if(rt == -1){\
        ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s",errno, strerror(errno));\
    }                  \
    m_listen_fds.insert(event->getFd());\
    DEBUGLOG("add event success, fd[%d]",event->getFd());\

#define DELETE_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd());\
    if (it == m_listen_fds.end()) {\
        return;\
    }\
    int op = EPOLL_CTL_DEL;\
    epoll_event tmp = event->getEpollEvent();\
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);\
    if (rt == -1) {\
        ERRORLOG("failed epoll_ctl when delete fd , errno=%d, error=%s",errno, strerror(errno));\
    }                     \
    m_listen_fds.erase(event->getFd());\
    DEBUGLOG("delete event success, fd[%d]",event->getFd());\

namespace rocket{
static int g_epoll_max_timeout = 10000;
static int g_epoll_max_events = 10;
static thread_local EventLoop* t_current_eventloop = NULL;
//线程局部静态变量，用来判断是否创建过Eventloop,确保一个线程只创建一个
EventLoop::EventLoop() {
    if(t_current_eventloop != NULL){
        ERRORLOG("failed to create event loop, this thread has created event loop");
        exit(0);
    }
    m_thread_id = getThreadID();

    m_epoll_fd = epoll_create(10);//Linux2.6后 size无影响

    if(m_epoll_fd == -1){
        ERRORLOG("failed to create event loop, epoll_create error, error info[%d]",errno);
        exit(0);
    }

    initWakeUpFdEvent();//wakeup Fd相关操作
    initTimer();

    INFOLOG("succ create event loop in thread %d", m_thread_id);
    t_current_eventloop = this;
    //当前线程中eventloop对象的指针
}

EventLoop::~EventLoop() {
    close(m_epoll_fd);
    if(m_wakeup_fd_event){
        delete m_wakeup_fd_event;
        m_wakeup_fd_event = NULL;
    }

    if(m_timer){
        delete m_timer;
        m_timer = NULL;
    }
}

void EventLoop::initWakeUpFdEvent(){
    m_wakeup_fd = eventfd(0,EFD_NONBLOCK);
    if(m_wakeup_fd < 0){
        ERRORLOG("failed to create event loop, eventfd error, error info[%d]",errno);
        exit(0);
    }
    m_wakeup_fd_event = new WakeUpEvent(m_wakeup_fd);//
    DEBUGLOG("wakeup fd = %d",m_wakeup_fd_event->getFd());
    m_wakeup_fd_event->listen(FdEvent::IN_EVENT,[this](){
        char buf[8];
        //不等于-1说明有数据可以读 当错误号等于EAGAIN没有数据可以读
        while(read(m_wakeup_fd,buf,8) != -1 && errno != EAGAIN){
        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
    });

    addEpollEvent(m_wakeup_fd_event);
}

void EventLoop::initTimer() {
    m_timer = new Timer();
    addEpollEvent(m_timer);
}

void EventLoop::addTimerEvent(TimerEvent::s_ptr event){
    //Eventloop中添加定时事件 通过 将事件添加到容器中实现
    m_timer->addTimerEvent(event);//向定时器容器中添加定时事件，定时器容器是被eventloop监听的
}

void EventLoop::loop() {
    while(!m_stop_flag){
        ScopeMutex<Mutex> lock(m_mutex);
        std::queue<std::function<void()>> tmp_tasks;
        m_pending_tasks.swap(tmp_tasks);
        lock.unlock();

        while(!tmp_tasks.empty()){
            std::function<void()> cb = tmp_tasks.front();//模板编程，看functional,front()取出，再加()执行任务
            tmp_tasks.pop();
            if(cb){
                cb();
            }
        }

        //如果有定时任务需要执行，那么执行
        //1.怎么判断定时任务怎么执行？now()>=TimerEvent.arrive_time
        //2.arrive_time 如何让eventloop监听？

        //1. 取得下次定时任务的时间，与设定time_out取较大值，即下次定时任务时间超过1s就取定时任务时间为超时时间，否则取1s
        //2. 调用epoll_wait等待事件发生，超时时间为上述 time_out

        int time_out = g_epoll_max_timeout;// max(1000, getNextTimeCallback());
        epoll_event result_events[g_epoll_max_events];
//        DEBUGLOG("now begin to epoll_wait");
        int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, time_out);
//        DEBUGLOG("now end epoll_wait, rt = %d",rt);
        if(rt < 0){
            ERRORLOG("epoll_wait error error=%d",errno);
        }else{
            for(int i = 0; i<rt; ++i){
                epoll_event trigger_event = result_events[i];
                FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr);
                if(fd_event == NULL){
                    ERRORLOG("fd_event = NULL, continue");
                    continue;
                }
                if(trigger_event.events & EPOLLIN){
                    DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::TriggerEvent::IN_EVENT));
                }
                if(trigger_event.events & EPOLLOUT){
                    DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::TriggerEvent::OUT_EVENT));
                }

                if(trigger_event.events & EPOLLERR){
                    DEBUGLOG("fd %d trigger EPOLLERROR event", fd_event->getFd());
                    deleteEpollEvent(fd_event);
                    if(fd_event->handler(FdEvent::ERROR_EVENT) != nullptr){
                        DEBUGLOG("fd %d add error callback", fd_event->getFd());
                        addTask(fd_event->handler(FdEvent::OUT_EVENT));
                    }
                }
            }
        }
    }
}

void EventLoop::wakeup() {
    m_wakeup_fd_event->wakeup();
}

void EventLoop::dealWakeup() {

}

void EventLoop::stop() {
    m_stop_flag = true;
}

void EventLoop::addEpollEvent(FdEvent* event){
    if(isInLoopThread()) {
        ADD_TO_EPOLL();
//        auto it = m_listen_fds.find(event->getFd());
//        int op = EPOLL_CTL_ADD;
//        if(it != m_listen_fds.end()){
//            op = EPOLL_CTL_MOD;
//        }
//        epoll_event tmp = event->getEpollEvent();
//        INFOLOG("epoll_event.events = %d, fd = %d",(int)tmp.events, event->getFd());
//        int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp);
//        if(rt == -1){
//            ERRORLOG("failed epoll_ctl when add fd, errno=%d, error=%s",errno, strerror(errno));
//        }
//        m_listen_fds.insert(event->getFd());
//        DEBUGLOG("add event success, fd[%d]",event->getFd());
    }else{
        //如果不在创建EventLoop的线程
        //lamda函数，将 ADD_TO_EPOLL封装为回调函数，等到原本的线程再执行
        auto cb = [this,event](){
            ADD_TO_EPOLL();
        };
        addTask(cb, true);//将其添加到EventLoop的任务队列，等回到原本线程再执行
    }
}

void EventLoop::deleteEpollEvent(FdEvent* event){
    if(isInLoopThread()){
        DELETE_TO_EPOLL();
    }else {
        auto cb = [this, event]() {
            DELETE_TO_EPOLL();
        };
        addTask(cb,true);
    }
}

bool EventLoop::isInLoopThread(){
    return getThreadID() == m_thread_id;
}

void EventLoop::addTask(std::function<void()> cb, bool is_wake_up ){
    //cb回调函数是一个任务
    ScopeMutex<Mutex> lock(m_mutex);
    m_pending_tasks.push(cb);
    lock.unlock();
    if(is_wake_up){//wakeup to deal task immediately
        wakeup();
    }
}



}

