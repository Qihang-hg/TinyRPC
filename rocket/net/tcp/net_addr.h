//
// Created by qihang on 23-8-27.
//

#ifndef TINYRPC_NET_ADDR_H
#define TINYRPC_NET_ADDR_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <memory>

namespace rocket{

//抽象类 封装地址  子类实现各种地址类
class NetAddr {
public:
    typedef std::shared_ptr<NetAddr> s_ptr;//放在父类，是为了 支持其他子类，除了IPNetAddr外，比如 UnixAddr

    virtual sockaddr* getSockAddr() = 0;

    virtual socklen_t getSockLen() = 0;

    virtual int getFamily() = 0;

    virtual std::string toString() = 0;

    virtual bool checkValid() = 0;

};

class IPNetAddr : public NetAddr{
public:
    IPNetAddr(const std::string& ip, uint16_t port);//通过ip端口构造
    IPNetAddr(const std::string& addr);//通过地址构造
    IPNetAddr(sockaddr_in addr);//ipv4结构体构造

    sockaddr* getSockAddr();

    socklen_t getSockLen();

    int getFamily();

    std::string toString();

    bool checkValid();//检验地址合法性

private:
    std::string m_ip;
    uint16_t m_port;
    sockaddr_in m_addr;//ipv4 addr struct
};

}


#endif //TINYRPC_NET_ADDR_H
