cmake_minimum_required(VERSION 3.10)

# Название проекта
project(DaemonMD5Sum)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавляем флаги компилятора
add_compile_options(-Wall -Werror)

# Подключаем файлы исходного кода
add_executable(daemon
    main.cpp
    daemon.cpp
    config.cpp
)

# Подключаем необходимые библиотеки (для OpenSSL)
find_package(OpenSSL REQUIRED)
target_link_libraries(daemon OpenSSL::SSL OpenSSL::Crypto)

# Указываем путь для поиска заголовочных файлов
include_directories(${CMAKE_SOURCE_DIR})

# Выводим сообщение при успешной сборке
message(STATUS "Project has been configured and built successfully.")
