cmake_minimum_required(VERSION 3.10)

project(DaemonProject)
include_directories("/usr/include/openssl")
link_directories("/usr/lib/x86_64-linux-gnu")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenSSL REQUIRED)

add_executable(daemon daemon.cpp config.cpp main.cpp)

target_link_libraries(daemon PRIVATE OpenSSL::SSL OpenSSL::Crypto)