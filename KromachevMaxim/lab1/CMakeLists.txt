cmake_minimum_required(VERSION 3.15)

project(Daemon VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Daemon
    main.cpp
    daemon.cpp
    logger.cpp
    daemon.h
    logger.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_library(CONFIG_LIBRARY config++)

if (CONFIG_LIBRARY)
    target_link_libraries(Daemon PRIVATE ${CONFIG_LIBRARY})
else()
    message(WARNING "Library config++ not found.")
endif()

set(INSTALL_PATH "/tmp")

install(TARGETS Daemon DESTINATION ${INSTALL_PATH})

