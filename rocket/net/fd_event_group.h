//
// Created by qihang on 23-8-31.
//

#ifndef TINYRPC_FD_EVENT_GROUP_H
#define TINYRPC_FD_EVENT_GROUP_H

#include <vector>
#include "fd_event.h"
#include "../common/mutex.h"
#include "../common/log.h"

namespace rocket{

class FdEventGroup {

public:
    FdEventGroup(int size);
    ~FdEventGroup();

    FdEvent* getFdEvent(int fd);
public:
    static FdEventGroup* GetFdEventGroup();

private:
    int m_size{0};
    std::vector<FdEvent*> m_fd_group;
    Mutex m_mutex;
};

}



#endif //TINYRPC_FD_EVENT_GROUP_H
