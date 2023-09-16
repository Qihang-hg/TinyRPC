//
// Created by qihang on 23-9-6.
//

#ifndef TINYRPC_ABSTRACT_PROTOCOL_H
#define TINYRPC_ABSTRACT_PROTOCOL_H

#include <memory>

namespace rocket{

class AbstractProtocol {
public:
    typedef std::shared_ptr<AbstractProtocol> s_ptr;

};


}



#endif //TINYRPC_ABSTRACT_PROTOCOL_H
