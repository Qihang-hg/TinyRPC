#include <iostream>
#include <unistd.h>
#include "rocket/common/log.h"

void* func(void*){
    int i = 20;
    while(i--){
        DEBUGLOG("this is thread debug %s", "func");
        INFOLOG("this is thread info %s", "func");
    }
    return NULL;
}

int main() {
    rocket::Config::SetGlobalConfig("../rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    pthread_t thread;
    pthread_create(&thread,NULL,&func,NULL);
    int i = 20;
    while(i--){
        DEBUGLOG("test debug log %s", "11");
        INFOLOG("test info log %s", "11");
    }
    pthread_join(thread,NULL);
    return 0;
}
