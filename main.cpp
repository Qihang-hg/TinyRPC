#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "rocket/common/log.h"
#include "rocket/net/fd_event.h"
#include "rocket/net/eventloop.h"

void* func(void*){
    int i = 20;
    while(i--){
        DEBUGLOG("this is thread debug %s", "func");
        INFOLOG("this is thread info %s", "func");
    }
    return NULL;
}

int test_main01() {
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

int test_main02(){
    rocket::Config::SetGlobalConfig("../rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();

    rocket::EventLoop* eventloop = new rocket::EventLoop();
    int  listenfd = socket(AF_INET, SOCK_STREAM,0);
    if(listenfd == -1){
        ERRORLOG("listenfd = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(12346);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);
    int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if(rt == -1){
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(listenfd,100);
    if(rt == -1){
        ERRORLOG("listen error");
        exit(1);
    }

    rocket::FdEvent event(listenfd);
    event.listen(rocket::FdEvent::IN_EVENT,[listenfd](){
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr,0, sizeof(peer_addr));
        int clientfd = accept(listenfd,reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);
        DEBUGLOG("success get client fd[%d] peer addr[%s:%d]",clientfd,inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    });
    eventloop->addEpollEvent(&event);
    eventloop->loop();
}

int main(){
    test_main02();
    return 0;
}
