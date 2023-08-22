//
// Created by qihang on 23-8-21.
//

#include <unistd.h>
#include "wakeup_fd_event.h"
#include "../common/log.h"

namespace rocket{

WakeUpEvent::WakeUpEvent(int fd) : FdEvent(fd) {

}
WakeUpEvent::WakeUpEvent(){

}
WakeUpEvent::~WakeUpEvent(){

}



void WakeUpEvent::wakeup(){
    char buf[8] = {'a'};
    int rt = write(m_fd,buf,8);
    if(rt != 8){
        ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
    }
    DEBUGLOG("success read 8 bytes");
}

}