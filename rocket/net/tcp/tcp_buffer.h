//
// Created by qihang on 23-8-27.
//

#ifndef TINYRPC_TCP_BUFFER_H
#define TINYRPC_TCP_BUFFER_H

#include <vector>
#include <memory>

namespace rocket{

class TcpBuffer {
public:
    typedef std::shared_ptr<TcpBuffer> s_ptr;

    TcpBuffer(int size);
    ~TcpBuffer();

    int readAble();//return the numer of able read bytes;
    int writeAble();//返回可写的字节数

    int readIndex();//获取read_index
    int writeIndex();

    void writeToBuffer(const char* buf, int size);//从buf向buffer中写入size个

    void readFromBuffer(std::vector<char>& re, int size);//从buffer中读出size个

    void resizeBuffer(int new_size);

    void adjustBuffer();//已读取字节过多时，调整将其覆盖，不扩容

    void moveReadIndex(int size);//手动调整读取指针
    void moveWriteIndex(int size);//
private:
    int m_read_index{0};
    int m_write_index{0};
    int m_size{0};

public:
    std::vector<char> m_buffer;
};

}



#endif //TINYRPC_TCP_BUFFER_H
