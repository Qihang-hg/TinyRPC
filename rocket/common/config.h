//
// Created by qihang on 23-8-13.
//

#ifndef TINYRPC_CONFIG_H
#define TINYRPC_CONFIG_H

#include <map>
#include <string>


namespace rocket{
    class Config {
    public:
        Config(const char* xmlfile);

        static  Config* GetGlobalConfig();
        static  void SetGlobalConfig(const char* xmlfile);

    public:
        std::string m_log_level;
    };
}



#endif //TINYRPC_CONFIG_H
