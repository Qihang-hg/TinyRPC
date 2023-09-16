//
// Created by qihang on 23-8-12.
//

#ifndef TINYRPC_LOG_H
#define TINYRPC_LOG_H

#include <string>
#include <queue>
#include <memory>
#include "config.h"
#include "mutex.h"



namespace rocket {
    //将字符数组str 以及其他可变参数  转为 格式化字符串
    template<typename ... Args>
    std::string formatString(const char *str, Args &&... args) {
        //写到空指针中 计算字符数量
        int size = snprintf(nullptr, 0, str, args...);
        std::string result;
        if (size > 0) {
            result.resize(size);
            snprintf(&result[0], size + 1, str, args...);
        }
        return result;
    }

//宏里面的logger不要new 后面没有析构，用栈上的对象。自动析构

#define DEBUGLOG(str, ...) \
      if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Debug)                        \
      {                        \
        rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Debug).toString() \
          +"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t" + rocket::formatString(str, ##__VA_ARGS__) +"\n");\
        rocket::Logger::GetGlobalLogger()->log();                                                  \
      }\

#define INFOLOG(str, ...) \
      if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Info)                        \
      {                        \
        rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Info).toString() \
          +"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+ rocket::formatString(str, ##__VA_ARGS__) +"\n");\
        rocket::Logger::GetGlobalLogger()->log();                                                  \
      }                        \

#define ERRORLOG(str, ...) \
      if(rocket::Logger::GetGlobalLogger()->getLogLevel()<=rocket::Error)                        \
      {                    \
        rocket::Logger::GetGlobalLogger()->pushLog(rocket::LogEvent(rocket::LogLevel::Error).toString() \
          +"["+std::string(__FILE__)+":"+std::to_string(__LINE__)+"]\t"+ rocket::formatString(str, ##__VA_ARGS__) +"\n");\
        rocket::Logger::GetGlobalLogger()->log();                                                                                                                \
      }\


    enum LogLevel {
        Unknown = 0,
        Debug = 1,
        Info = 2,
        Error = 3
    };

    class Logger{
    public:
        typedef std::shared_ptr<Logger> s_ptr;
        void pushLog(const std::string& msg);

        Logger(LogLevel level):m_set_level(level){}
        void log();
        LogLevel getLogLevel()const {
            return m_set_level;
        }
    public:
        static Logger* GetGlobalLogger();
        static void InitGlobalLogger();
    private:
        LogLevel m_set_level;
        std::queue<std::string> m_buffer;

        Mutex m_mutex;
    };

    std::string LogLevelToString(LogLevel level);
    LogLevel StringToLogLevel(const std::string & log_level);

    class LogEvent {
    public:
        LogEvent(LogLevel level):m_level(level){}

        std::string getFileName() const {
            return m_file_name;
        }

        LogLevel getLogLevel() const {
            return m_level;
        }

        std::string toString();

    private:
        std::string m_file_name; //文件名
        std::string m_file_line; //行号
        int m_pid;  //进程号
        int m_thread_id;    //线程号

        LogLevel m_level;

    };



}
#endif //TINYRPC_LOG_H
