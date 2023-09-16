//
// Created by qihang on 23-8-21.
//

#ifndef TINYRPC_EVENTLOOP_H
#define TINYRPC_EVENTLOOP_H

#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "../common/mutex.h"
#include "fd_event.h"
#include "wakeup_fd_event.h"
#include "timer.h"


namespace rocket{
class EventLoop {
public:
    EventLoop();

    ~EventLoop();

    void loop();

    void wakeup();

    void stop();

    void addEpollEvent(FdEvent* event);//判断是否已经存在，存在则修改
    void deleteEpollEvent(FdEvent* event);
    bool isInLoopThread();

    void addTask(std::function<void()> cb, bool is_wake_up = false);
    //把任务添加到pending队列，然后从epollwaite返回后自己执行，而不是由其他线程执行

    void addTimerEvent(TimerEvent::s_ptr event);
    bool isLooping();

public:
    static EventLoop* GetCurrentEventLoop();//获取当前线程中的EventLoop对象，如果没有就创建一个
private:
    void dealWakeup();
    void initWakeUpFdEvent();
    void initTimer();//初始化定时器容器，并在eventloop中监听
private:
    pid_t m_thread_id{0};

    int m_epoll_fd{0};

    int m_wakeup_fd{0};

    WakeUpEvent* m_wakeup_fd_event{NULL};

    bool m_stop_flag{false};

    std::set<int> m_listen_fds;

    std::queue<std::function<void()>> m_pending_tasks;//待执行的任务队列

    Mutex m_mutex;

    Timer* m_timer{NULL};//定时器容器

    bool m_is_looping{false};
};


}



#endif //TINYRPC_EVENTLOOP_H
