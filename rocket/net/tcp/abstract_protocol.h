//
// Created by qihang on 23-9-6.
//

#ifndef TINYRPC_ABSTRACT_PROTOCOL_H
#define TINYRPC_ABSTRACT_PROTOCOL_H

#include <memory>
#include <string>
#include "tcp_buffer.h"

namespace rocket{


/*
 * 为什么要用 enable_shared_from_this？
需要在类对象的内部中获得一个指向当前对象的 shared_ptr 对象。
如果在一个程序中，对象内存的生命周期全部由智能指针来管理。在这种情况下，要在一个类的成员函数中，对外部返回 this 指针就成了一个很棘手的问题。
 */
class AbstractProtocol: public std::enable_shared_from_this<AbstractProtocol>{
public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

    virtual ~AbstractProtocol(){}

    std::string getReqId(){
        return m_req_id;
    }

    void setReqId(const std::string& req_id){
        m_req_id = req_id;
    }

protected:
    std::string m_req_id; //请求号，唯一标识一个请求或者响应
};


}



#endif //TINYRPC_ABSTRACT_PROTOCOL_H
