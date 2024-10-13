#!/bin/sh

# Определяем начальную директорию
initial_dir=$(dirname "$(readlink -f "$0")")

# Проверка наличия CMake
if ! command -v cmake &> /dev/null; then
    echo "cmake is not installed. Please install cmake and try again."
    exit 1
fi

# Создаем папку build, если её нет
mkdir -p "$initial_dir/build"

# Переходим в папку build
cd "$initial_dir/build" || { echo "Failed to change to the build directory."; exit 1; }

# Запуск CMake для генерации Makefile
cmake ..
if [ $? -ne 0 ]; then
    echo "Error while running cmake."
    exit 1
fi

# Компиляция проекта с помощью make
make
if [ $? -ne 0 ]; then
    echo "Error while running make."
    exit 1
fi

# Возвращаемся в начальную директорию
cd "$initial_dir" || { echo "Failed to exit build directory."; exit 1; }

# Запускаем демон с конфигурационным файлом
sudo "$initial_dir/build/daemon" "$initial_dir/config.cfg"

# Удаляем папку build после выполнения
sudo rm -rf "$initial_dir/build"
