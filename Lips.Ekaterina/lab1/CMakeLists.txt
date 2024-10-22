cmake_minimum_required(VERSION 3.10)
project(lab1)

set(CMAKE_CXX_STANDARD 17)

add_executable(lab1 main.cpp daemon.cpp)
target_compile_options(lab1 PRIVATE -Wall -Wextra -Wpedantic -Werror)