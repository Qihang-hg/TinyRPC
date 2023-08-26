//
// Created by qihang on 23-8-12.
//
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include "util.h"


namespace rocket{
static int g_pid = 0;
static thread_local int g_thread_id=0;
pid_t getPid() {
    if(g_pid !=0 ){
        return g_pid;
    }
    return getpid();
}

pid_t getThreadID(){
    if(g_thread_id!=0){
        return g_thread_id;
    }
    return syscall(SYS_gettid);
    //进程p1中的线程pt1要与进程p2中的线程pt2通信怎么办，进程id不可以，线程id又可能重复，
    // 所以这里会有一个真实的线程id唯一标识，tid
    // linux下的系统调用syscall(SYS_gettid)来获得
}

int64_t getNowMs(){
    timeval val;
    gettimeofday(&val,NULL);
    return val.tv_sec*1000 + val.tv_usec/1000;  //ms
}

}