//
// Created by qihang on 23-9-16.
//

#ifndef TINYRPC_ABSTRACT_CODER_H
#define TINYRPC_ABSTRACT_CODER_H

#include <vector>
#include "tcp/tcp_buffer.h"
#include "tcp/abstract_protocol.h"

namespace rocket{


class AbstractCoder {
public:

    virtual ~AbstractCoder(){}

    //将message对象转换为字节流，写入到buffer
    virtual void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) = 0;

    //将buffer对象里面的字节流，转换为message对象
    virtual void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) = 0;

};

}



#endif //TINYRPC_ABSTRACT_CODER_H
