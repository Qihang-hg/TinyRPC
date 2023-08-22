//
// Created by qihang on 23-8-21.
//

#ifndef TINYRPC_WAKEUP_FD_EVENT_H
#define TINYRPC_WAKEUP_FD_EVENT_H

#include "fd_event.h"

namespace rocket{

class WakeUpEvent : public FdEvent{
public:
    WakeUpEvent(int fd);
    WakeUpEvent();
    ~WakeUpEvent();


    void wakeup();
private:


};


}



#endif //TINYRPC_WAKEUP_FD_EVENT_H
