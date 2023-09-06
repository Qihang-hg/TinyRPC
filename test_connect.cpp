//
// Created by qihang on 23-8-31.
//

#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <memory>
#include "rocket/common/log.h"
#include "rocket/common/config.h"

void test_main_connect(){
    //调用connect连接server
    //write 一个字符
    //等待read返回结果
    int  fd = socket(AF_INET, SOCK_STREAM,0);
    if(fd <= 0){
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

    std::string msg = "hello,world";
//    char msg[128] = "hello,world";
    rt = write(fd, msg.c_str(), msg.length());
//    rt = write(fd, msg, 128);

    DEBUGLOG("success write %d bytes, [%s]",rt,msg.c_str());


    char buf[128];
    memset(buf,0,sizeof(buf));
    int re = read(fd, buf, ssize_t(128));
    DEBUGLOG("success read %d bytes, [%s]", re, std::string(buf).c_str());
}

int main(){
    rocket::Config::SetGlobalConfig("../rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
    test_main_connect();
    return 0;
}