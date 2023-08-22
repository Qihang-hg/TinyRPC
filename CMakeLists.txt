cmake_minimum_required(VERSION 3.25)
project(TinyRPC)

set(CMAKE_CXX_STANDARD 11)



add_executable(TinyRPC main.cpp rocket/common/log.cpp rocket/common/log.h rocket/common/util.cpp
        rocket/common/util.h rocket/testcases/test_log.cpp rocket/common/config.cpp rocket/common/config.h
        rocket/common/tinyxml/tinystr.cpp rocket/common/tinyxml/tinystr.h
        rocket/common/tinyxml/tinyxml.cpp rocket/common/tinyxml/tinyxml.h
        rocket/common/tinyxml/tinyxmlerror.cpp rocket/common/tinyxml/tinyxmlparser.cpp
        rocket/common/mutex.h rocket/net/eventloop.cpp rocket/net/eventloop.h rocket/net/fd_event.cpp rocket/net/fd_event.h rocket/net/wakeup_fd_event.cpp rocket/net/wakeup_fd_event.h)


