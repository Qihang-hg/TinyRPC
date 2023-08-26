//
// Created by qihang on 23-8-23.
//

#ifndef TINYRPC_TIMER_EVENT_H
#define TINYRPC_TIMER_EVENT_H
#include <functional>
#include <memory>
namespace rocket{
class TimerEvent {

public:
    typedef  std::shared_ptr<TimerEvent> s_ptr;//智能指针

    TimerEvent(int interval, bool is_repeated, std::function<void()> cb);
    ~TimerEvent();

    int64_t getArriveTime() const{
        return m_arrive_time;
    }

    void setCanceled(bool value){
        m_is_canceled = value;
    }

    bool isCanceled(){
        return m_is_canceled;
    }
    bool isRepeated(){
        return  m_is_repeated;
    }

    std::function<void()> getCallBack(){
        return m_task;
    }

    void resetArriveTime();

private:
    int64_t m_arrive_time;  //ms 到达时间
    int64_t m_interval;     //ms 到下次触发的时间间隔
    bool m_is_repeated {false};
    bool m_is_canceled {false};

    std::function<void()> m_task;
};

}


#endif //TINYRPC_TIMER_EVENT_H
