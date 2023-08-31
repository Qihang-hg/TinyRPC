cmake_minimum_required(VERSION 3.25)
project(TinyRPC)

set(CMAKE_CXX_STANDARD 11)



add_executable(TinyRPC main.cpp rocket/common/log.cpp rocket/common/log.h rocket/common/util.cpp
        rocket/common/util.h rocket/testcases/test_log.cpp rocket/common/config.cpp rocket/common/config.h
        rocket/common/tinyxml/tinystr.cpp rocket/common/tinyxml/tinystr.h
        rocket/common/tinyxml/tinyxml.cpp rocket/common/tinyxml/tinyxml.h
        rocket/common/tinyxml/tinyxmlerror.cpp rocket/common/tinyxml/tinyxmlparser.cpp
        rocket/common/mutex.h
        rocket/net/eventloop.cpp rocket/net/eventloop.h
        rocket/net/fd_event.cpp rocket/net/fd_event.h
        rocket/net/wakeup_fd_event.cpp rocket/net/wakeup_fd_event.h
        rocket/net/timer.cpp rocket/net/timer.h
        rocket/net/timer_event.cpp rocket/net/timer_event.h
        rocket/net/io_thread.cpp rocket/net/io_thread.h
        rocket/net/io_thread_group.cpp rocket/net/io_thread_group.h
        rocket/net/tcp/tcp_buffer.cpp rocket/net/tcp/tcp_buffer.h
        rocket/net/tcp/tcp_acceptor.cpp rocket/net/tcp/tcp_acceptor.h
        rocket/net/tcp/net_addr.cpp rocket/net/tcp/net_addr.h
        rocket/net/tcp/tcp_server.cpp rocket/net/tcp/tcp_server.h
        rocket/net/fd_event_group.cpp rocket/net/fd_event_group.h
        rocket/net/tcp/tcp_connection.cpp rocket/net/tcp/tcp_connection.h)

add_executable(Test test_connect.cpp rocket/common/log.cpp rocket/common/log.h rocket/common/util.cpp
        rocket/common/util.h rocket/testcases/test_log.cpp rocket/common/config.cpp rocket/common/config.h
        rocket/common/mutex.h
        rocket/common/tinyxml/tinystr.cpp rocket/common/tinyxml/tinystr.h
        rocket/common/tinyxml/tinyxml.cpp rocket/common/tinyxml/tinyxml.h
        rocket/common/tinyxml/tinyxmlerror.cpp rocket/common/tinyxml/tinyxmlparser.cpp)
