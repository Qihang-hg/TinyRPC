//
// Created by qihang on 23-8-26.
//

#ifndef TINYRPC_IO_THREAD_GROUP_H
#define TINYRPC_IO_THREAD_GROUP_H

#include <vector>
#include "../common/log.h"
#include "io_thread.h"

namespace rocket{

class IOThreadGroup{
public:
    IOThreadGroup(int size);
    ~IOThreadGroup();

    void start();

    IOThread* getIOThread();//获取一个可用的io线程

    void join();
private:
    size_t m_size{0};
    std::vector<IOThread*> m_io_thread_groups;

    int m_index{0}; //当前应该获取的io线程的下标
};

}



#endif //TINYRPC_IO_THREAD_GROUP_H
