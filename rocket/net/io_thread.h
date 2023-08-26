//
// Created by qihang on 23-8-26.
//

#ifndef TINYRPC_IO_THREAD_H
#define TINYRPC_IO_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include "eventloop.h"

namespace rocket{

class IOThread {
public:
    IOThread();
    ~IOThread();

    EventLoop* getEventLoop();

    void start();

    void join();
public:
    static void* Main(void* arg);//线程的回调函数
private:
    pid_t m_thread_id{-1};   //线程号
    pthread_t m_thread{0}; //线程句柄
    EventLoop* m_event_loop{NULL};  //当前io线程的eventloop对象

    sem_t m_init_semaphore; //初始化时用的用信号，保证loop前置执行
    sem_t m_start_semaphore;//启动loop信号量
};


}




#endif //TINYRPC_IO_THREAD_H
