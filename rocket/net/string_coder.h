//
// Created by qihang on 23-9-16.
//

#ifndef TINYRPC_STRING_CODER_H
#define TINYRPC_STRING_CODER_H

#include "abstract_coder.h"

namespace rocket{

class StringProtocol : public AbstractProtocol{

public:
    std::string info;
};

class StringCoder: public AbstractCoder{
    //将message对象转换为字节流，写入到buffer
    void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) {

        for(int i = 0; i<messages.size(); ++i){
            std::shared_ptr<StringProtocol> msg = std::dynamic_pointer_cast<StringProtocol>(messages[i]);
            out_buffer->writeToBuffer(msg->info.c_str(), msg->info.length());
        }

    }

    //将buffer对象里面的字节流，转换为message对象
    void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) {
        std::vector<char> re;
        buffer->readFromBuffer(re, buffer->readAble());
        std::string info = "";
        for(size_t i = 0; i<re.size(); ++i){
            info += re[i];
        }

        //没有对收到的字节流做解析
        //暂时直接用 声明一个 Protocol对象作为 message。用于测试
        std::shared_ptr<StringProtocol> msg = std::make_shared<StringProtocol>();
        msg->info = info;
        msg->setReqId("123456");//需要自己设置，返回后才能匹配确认
        out_messages.push_back(msg);
    }
};

}



#endif //TINYRPC_STRING_CODER_H
