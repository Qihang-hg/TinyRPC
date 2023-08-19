//
// Created by qihang on 23-8-13.
//
//#include <tinyxml/tinyxml.h>
#include "tinyxml/tinyxml.h"
#include "config.h"

#define READ_XML_NODE(name, parent) \
    TiXmlElement* name##_node = parent->FirstChildElement(#name); \
    if(!name##_node){             \
        printf("Start rocket server error, failed to read config file of [%s]",#name);\
        exit(0);                  \
    }                             \
/*
 * 在宏变量前面加一个#号，表示把宏变量转变为字符串；两个#号并列（即##），表示将宏中的两个变量连接起来。
 * 通过 ## 可将两个宏展开成一个，即将两者进行了拼接，
 * 这种操作叫"token pasting"，或"token concatenation"，就是拼接嘛。
 * 宏拼接一般用在需要拼接的宏是来自宏参数的情况
*/
#define READ_STR_FROM_XML_NODE(name,parent) \
    TiXmlElement* name##_node = parent->FirstChildElement(#name); \
    if(!name##_node || !name##_node->GetText()){                        \
        printf("Start rocket server error, failed to read node [%s]", #name);\
        exit(0);                            \
    }                                       \
    std::string name##_str = std::string(name##_node->GetText());\

namespace rocket{

    static Config* g_config = NULL;
    Config* Config::GetGlobalConfig(){
        return g_config;
    }
    void Config::SetGlobalConfig(const char* xmlfile){
        if(g_config == NULL){
            g_config = new Config(xmlfile);
        }
    }

    Config::Config(const char* xmlfile){
        TiXmlDocument* xml_document = new TiXmlDocument();
        //从配置文件读取配置
        bool rt = xml_document->LoadFile(xmlfile) ;
        if(!rt){
            printf("Start rocket server error, failed to read config file %s, error info[%s]\n",xmlfile,xml_document->ErrorDesc());
            exit(0);
        }
        READ_XML_NODE(root, xml_document);
        READ_XML_NODE(log, root_node);
        READ_STR_FROM_XML_NODE(log_level, log_node);

        m_log_level = log_level_str;
    }





}

// READ_XML_NODE(root, xml_document);
//        TiXmlElement* root_node = xml_document->FirstChildElement("root");
//        if(!root_node){
//            printf("Start rocket server error, failed to read [%s]\n","root");
//            exit(0);
//        }

//        READ_XML_NODE(log, root_node);
//        TiXmlElement* log_node = root_node->FirstChildElement("log");
//        if(!log_node){
//            printf("Start rocket server error, failed to read [%s]\n","log");
//            exit(0);
//        }

// READ_STR_FROM_XML_NODE(log_level, log_node);
//        TiXmlElement* log_level_node = log_node->FirstChildElement("log_level");
//        if(!log_level_node){
//            printf("Start rocket server error, failed to read config file %s\n","log_level");
//            exit(0);
//        }
//        std::string log_level = std::string(log_level_node->GetText());