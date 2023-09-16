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
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/string_coder.h"
#include "rocket/net/tcp/abstract_protocol.h"


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

void test_tcp_client(){
    rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1",12345);
    rocket::TcpClient client(addr);
    client.connect([addr, &client](){
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        std::shared_ptr<rocket::StringProtocol> message = std::make_shared<rocket::StringProtocol>();
        message->info = "hello rocket";
        message->setReqId("123456");

        client.writeMessage(message, [](rocket::AbstractProtocol::s_ptr msg_ptr){
            DEBUGLOG("send message[1] success")
        });

        client.readMessage("123456", [](rocket::AbstractProtocol::s_ptr msg_ptr){
            //父类智能指针 向 子类指针转换
            /*2.dynamic_cast 一般用于有继承关系的类之间的向下转换。
             *3.dynamic_pointer_cast  当指针是智能指针时候，向下转换，用dynamic_Cast 则编译不能通过，此时需要使用dynamic_pointer_cast。
             *
             * */
            std::shared_ptr<rocket::StringProtocol> message = std::dynamic_pointer_cast<rocket::StringProtocol>(msg_ptr);
            DEBUGLOG("req_id[%s], get response:%s", message->getReqId().c_str(), message->info.c_str());
        });

        client.writeMessage(message, [](rocket::AbstractProtocol::s_ptr msg_ptr){
            DEBUGLOG("send message[2] success")
        });



        client.readMessage("123456", [](rocket::AbstractProtocol::s_ptr msg_ptr){
            //父类智能指针 向 子类指针转换
            /*2.dynamic_cast 一般用于有继承关系的类之间的向下转换。
             *3.dynamic_pointer_cast  当指针是智能指针时候，向下转换，用dynamic_Cast 则编译不能通过，此时需要使用dynamic_pointer_cast。
             *
             * */
            std::shared_ptr<rocket::StringProtocol> message = std::dynamic_pointer_cast<rocket::StringProtocol>(msg_ptr);
            DEBUGLOG("req_id[%s], get response:%s", message->getReqId().c_str(), message->info.c_str());
        });
    });
}

int main(){
    rocket::Config::SetGlobalConfig("../rocket/conf/rocket.xml");
    rocket::Logger::InitGlobalLogger();
//    test_main_connect();

    test_tcp_client();
    return 0;
}