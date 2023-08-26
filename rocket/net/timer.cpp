//
// Created by qihang on 23-8-23.
//

#include <sys/timerfd.h>
#include <string.h>
#include "timer.h"
#include "../common/log.h"
#include "../common/util.h"

namespace rocket{

Timer::Timer(): FdEvent(){
    //定时器容器文件描述符 ,此时未设置到达时间
    //当容器中 第一个元素更新或 添加第一个元素时 设置超时时间
    m_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    //CLOCK_MONOTONIC 表示自过去某个任意固定点经过的绝对挂钟时间
    //TFD_NONBLOCK ：设置该文件描述符为非阻塞模式
    //TFD_CLOEXEC 表示当程序执行exec函数时本fd将被系统自动关闭,表示不传递给exec创建的新进程
    DEBUGLOG("timer fd = %d", m_fd);

    //把fd的可读事件以及 回调函数传入。将onTimer成员函数绑定作为 function对象作为回调
    listen(FdEvent::IN_EVENT, std::bind(&Timer::onTimer,this));
    DEBUGLOG("add timer fd[%d] to eventloop", m_fd);
    //把定时器容fd的可读事件放到listen上监听
}

Timer::~Timer(){}

void Timer::resetArriveTime(){
    ScopeMutex<Mutex> lock(m_mutex);
    auto tmp = m_pending_events;
    lock.unlock();

    if(tmp.size() == 0){
        return;
    }

    int64_t now = getNowMs();

    auto it = tmp.begin();
    int64_t interval = 0;
    if(it->second->getArriveTime() > now){
        interval = it->second->getArriveTime() - now;
    }else{
        interval = 100;
    }

    /*
struct itimerspec
{
    struct timespec it_interval;   //间隔时间
    struct timespec it_value;      //第一次到期时间
};

struct timespec
{
    time_t tv_sec;    //秒
    long tv_nsec;    //纳秒
};*/
    timespec ts;
    memset(&ts,0, sizeof(ts));
    ts.tv_sec = interval/1000;
    ts.tv_nsec = (interval%1000)*1000000;

    itimerspec value;
    memset(&value,0, sizeof(value));
    value.it_value = ts;

    //采用相对时间进行设置 flags=0
    int rt = timerfd_settime(m_fd,0,&value, NULL);
    if(rt!=0){
        ERRORLOG("timerfd_settime error, errno = %d, strerr = %s", errno, strerror(errno));
    }
    DEBUGLOG("timer reset to %lld", now + interval);

}

//向定时器容器中添加 定时事件
void Timer::addTimerEvent(TimerEvent::s_ptr event){
    //用于判断是否需要重新设置 定时器容器fd的超时时间
    bool is_reset_timerfd = false;

    ScopeMutex<Mutex> lock(m_mutex);
    //需要重置 到达时间 的情况：
    //如果定时器容器为空，需要设置超时时间，不然不会触发
    //如果添加到容器中的事件到达时间 小于 容器中所有事件的到达时间，则定时触发时间需要修改。否则容器的触发事件还是之前第一个事件的时间
    if(m_pending_events.empty()){
        is_reset_timerfd = true;
    }else{
        auto it =m_pending_events.begin();
        if((*it).second->getArriveTime() > event->getArriveTime()){
            is_reset_timerfd = true;
        }
    }
    m_pending_events.emplace(event->getArriveTime(), event);
    lock.unlock();

    if(is_reset_timerfd){
        resetArriveTime();
    }
}

void Timer::deleteTimerEvent(TimerEvent::s_ptr event){
    event->setCanceled(true);

    ScopeMutex<Mutex> lock(m_mutex);
    auto begin = m_pending_events.lower_bound(event->getArriveTime());//第一个小于等于给定事件事件的
    auto end = m_pending_events.upper_bound(event->getArriveTime());
    //multiset::upper_bound()是C++ STL中的内置函数，该函数返回一个迭代器，该迭代器指向刚好大于key的下一个元素

    auto it = begin;
    for(it = begin; it != end; ++it){
        if(it->second == event){
            break;
        }
    }
    if(it != end){
        m_pending_events.erase(it);
    }
    lock.unlock();
    DEBUGLOG("success delete TimerEvent at arrive time %lld",event->getArriveTime());


}

void Timer::onTimer() {
    //处理缓冲区数据 防止下一次继续触发可读事件
    DEBUGLOG("onTimer()");
    char buf[8];
    while(true){
        if((read(m_fd,buf,8) == -1) && errno  == EAGAIN){
            break;
        }
    }

    //执行定时任务
    int64_t  now = getNowMs();

    std::vector<TimerEvent::s_ptr> tmps;
    std::vector<std::pair<int64_t, std::function<void()>>> tasks;

    ScopeMutex<Mutex> lock(m_mutex);
    auto it = m_pending_events.begin();

    for(it = m_pending_events.begin(); it!= m_pending_events.end(); ++it){
        if((*it).first <= now){
            if(!(*it).second->isCanceled()){
                tmps.push_back((*it).second);
                tasks.push_back(std::make_pair((*it).second->getArriveTime(), (*it).second->getCallBack()));
            }
        }else{
            break;
        }
    }

    m_pending_events.erase(m_pending_events.begin(), it);
    lock.unlock();

    //需要把重复的event事件 再添加
    for(auto i = tmps.begin(); i!= tmps.end(); ++i){
        if((*i)->isRepeated()){
            //调整arriveTime
            (*i)->resetArriveTime();
            addTimerEvent(*i);
        }
    }

    resetArriveTime();

    for(auto i: tasks){
        if(i.second){
            i.second();
        }
    }
}

}
