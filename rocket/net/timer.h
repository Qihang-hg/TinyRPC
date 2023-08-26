//
// Created by qihang on 23-8-23.
//

#ifndef TINYRPC_TIMER_H
#define TINYRPC_TIMER_H

#include <map>
#include "fd_event.h"
#include "../common/mutex.h"
#include "timer_event.h"

//timer要能被eventloop监听，应该也是一个事件类，继承fdevent
//作为容器 容纳定时事件 类成员含有timer_event（包含定时事件属性）
namespace rocket{

class Timer : public FdEvent{
public:
    Timer();
    ~Timer();

    void addTimerEvent(TimerEvent::s_ptr event);//添加事件到容器中

    void deleteTimerEvent(TimerEvent::s_ptr event); //从容器中删除事件

    void onTimer(); //监听Timer套接字IO的回调函数,eventloop会执行这个函数

private:
    void resetArriveTime();

private:
    std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;  //封装的定时事件set
    Mutex m_mutex;
};

}



#endif //TINYRPC_TIMER_H
