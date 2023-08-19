//
// Created by qihang on 23-8-19.
//

#ifndef TINYRPC_MUTEX_H
#define TINYRPC_MUTEX_H

#include <pthread.h>

namespace rocket{

/*互斥锁采用资源获取即初始化——RAII
 * 用锁时创建ScopeMutex，解锁时候销毁ScopeMutex
 * */
template<class T>
class ScopeMutex {
public:
    ScopeMutex(T& mutex):m_mutex(mutex){
        m_mutex.lock();
        m_is_lock = true;
    }
    ~ScopeMutex(){
        m_mutex.unlock();
        m_is_lock = false;
    }

    void lock(){
        if(!m_is_lock){
            m_mutex.lock();
        }
    }
    void unlock(){
        if(m_is_lock){
            m_mutex.unlock();
        }
    }
private:
    T& m_mutex;
    bool m_is_lock{false};
};

class Mutex{
public:
    Mutex(){
        pthread_mutex_init(&m_mutex,NULL);
    }
    ~Mutex(){
        pthread_mutex_destroy(&m_mutex);
    }

    void lock(){
        pthread_mutex_lock(&m_mutex);
    }
    void unlock(){
        pthread_mutex_unlock(&m_mutex);
    }
private:
    pthread_mutex_t m_mutex;
};

}



#endif //TINYRPC_MUTEX_H
