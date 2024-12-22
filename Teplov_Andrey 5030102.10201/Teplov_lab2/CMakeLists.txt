cmake_minimum_required(VERSION 3.5)

project(DamirAsanov)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(CMAKE_AUTOMOC ON) # for Q_OBJECT

############ sockets ############
set(SOURCES_SOCK
    host_sock.cpp
    HostUtils/HostWindow.cpp
    ClientUtils/ClientWindow.cpp
    conn_sock.cpp
    semaphore.cpp
)

set(HEADERS_SOCK
    HostUtils/HostWindow.hpp
    ClientUtils/ClientWindow.hpp
    conn_sock.hpp
    semaphore.hpp
    logger.hpp
)

add_executable(host_sock ${SOURCES_SOCK} ${HEADERS_SOCK})
target_link_libraries(host_sock Qt5::Widgets)
target_include_directories(host_sock PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

############ queue ############
set(SOURCES_MQ
    host_mq.cpp
    HostUtils/HostWindow.cpp
    ClientUtils/ClientWindow.cpp
    conn_mq.cpp
    semaphore.cpp
)

set(HEADERS_MQ
    HostUtils/HostWindow.hpp
    ClientUtils/ClientWindow.hpp
    conn_mq.hpp
    semaphore.hpp
    logger.hpp
)

add_executable(host_mq ${SOURCES_MQ} ${HEADERS_MQ})
target_link_libraries(host_mq Qt5::Widgets)
target_include_directories(host_mq PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

############ fifo ############
set(SOURCES_FIFO
    host_fifo.cpp
    HostUtils/HostWindow.cpp
    ClientUtils/ClientWindow.cpp
    conn_fifo.cpp
    semaphore.cpp
)

set(HEADERS_FIFO
    HostUtils/HostWindow.hpp
    ClientUtils/ClientWindow.hpp
    conn_fifo.hpp
    semaphore.hpp
    logger.hpp
)

add_executable(host_fifo ${SOURCES_FIFO} ${HEADERS_FIFO})
target_link_libraries(host_fifo Qt5::Widgets)
target_include_directories(host_fifo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

