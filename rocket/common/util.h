//
// Created by qihang on 23-8-12.
//

#ifndef TINYRPC_UTIL_H
#define TINYRPC_UTIL_H
#include <sys/types.h>
#include <unistd.h>

namespace rocket{
pid_t getPid();

pid_t getThreadID();

int64_t getNowMs();

}



#endif //TINYRPC_UTIL_H
