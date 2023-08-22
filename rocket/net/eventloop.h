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

namespace rocket{
class EventLoop {
public:
    EventLoop();

    ~EventLoop();

    void loop();

    void wakeup();

    void stop();

    void addEpollEvent(FdEvent* event);
    void deleteEpollEvent(FdEvent* event);
    bool isInLoopThread();

    void addTask(std::function<void()> cb, bool is_wake_up = false);
    //把任务添加到pending队列，然后从epollwaite返回后自己执行，而不是由其他线程执行

private:
    void dealWakeup();
    void initWakeUpFdEvent();
private:
    pid_t m_thread_id{0};

    int m_epoll_fd{0};

    int m_wakeup_fd{0};

    WakeUpEvent* m_wakeup_fd_event{NULL};

    bool m_stop_flag{false};

    std::set<int> m_listen_fds;

    std::queue<std::function<void()>> m_pending_tasks;//待执行的任务队列

    Mutex m_mutex;
};


}



#endif //TINYRPC_EVENTLOOP_H
