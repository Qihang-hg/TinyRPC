//
// Created by qihang on 23-8-23.
//

#include "../common/log.h"
#include "../common/util.h"
#include "timer_event.h"


namespace rocket{
TimerEvent::TimerEvent(int interval, bool is_repeated, std::function<void()> cb):
    m_interval(interval),m_is_repeated(is_repeated),m_task(cb)
{
    resetArriveTime();
}

TimerEvent::~TimerEvent(){}


void TimerEvent::resetArriveTime(){
    m_arrive_time = getNowMs() + m_interval;
    DEBUGLOG("success create timer event, will execute at[%lld]", m_arrive_time);
}

}