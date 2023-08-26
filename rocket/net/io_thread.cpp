//
// Created by qihang on 23-8-26.
//

#include <pthread.h>
#include <assert.h>
#include "io_thread.h"
#include "../common/log.h"
#include "../common/util.h"

namespace rocket{

IOThread::IOThread() {
    int rt = sem_init(&m_init_semaphore,0,0);
    assert(rt == 0);
    rt = sem_init(&m_start_semaphore,0,0);
    assert(rt == 0);

    pthread_create(&m_thread,NULL,&IOThread::Main, this);

    //wait,直到新线程执行完Main函数的前置，loop函数前,才能构造完成
    sem_wait(&m_init_semaphore);
    DEBUGLOG("IOThread [%d] cread success", m_thread_id);
}

IOThread::~IOThread() {
    m_event_loop->stop();
    sem_destroy(&m_init_semaphore);
    sem_destroy(&m_start_semaphore);

    pthread_join(m_thread, NULL);   //等待线程结束

    if(m_event_loop != NULL){
        delete m_event_loop;
        m_event_loop = NULL;
    }
}

//为了确保io线程的eventloop能顺利开启，需要信号量做线程同步。避免 main函数执行到一半
//loop还没开启就被切换，然后访问其main出错
void *IOThread::Main(void *arg) {
    IOThread* thread = static_cast<IOThread*>(arg);

    thread->m_event_loop = new EventLoop();
    thread->m_thread_id = getThreadID();
    //前面的前置执行完，再唤醒等待的线程。避免线程属性未确定好时被使用
    sem_post(&thread->m_init_semaphore);
    DEBUGLOG("IOThread [%d] create, wait init sem", thread->m_thread_id);
    //让IO线程等待，直到设置完eventloop的相关事件后，再由我们主动启动
    DEBUGLOG("IOThread [%d] create, wait start sem", thread->m_thread_id);
    sem_post(&thread->m_start_semaphore);

    DEBUGLOG("IOThread [%d] start loop", thread->m_thread_id);
    thread->m_event_loop->loop();
    DEBUGLOG("IOThread [%d] end loop", thread->m_thread_id);
}

EventLoop* IOThread::getEventLoop(){
    return m_event_loop;
}

void IOThread::start() {
    DEBUGLOG("Now invoke IOThread [%d]", m_thread_id)
    sem_post(&m_start_semaphore);
}

void IOThread::join() {
    pthread_join(m_thread,NULL);
}

};
