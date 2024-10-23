cmake_minimum_required(VERSION 3.10)
project(lab1_DAEMON)

set(CMAKE_CXX_STANDARD 17)

set(HEADERS Daemon.h configFile.h)
set(SOURCES main.cpp Daemon.cpp configFile.cpp)

add_executable(lab1_DAEMON ${HEADERS} ${SOURCES})
target_compile_options(lab1_DAEMON PRIVATE -Wall -Wextra -Wpedantic -Werror)