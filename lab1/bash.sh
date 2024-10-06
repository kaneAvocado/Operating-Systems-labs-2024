#!/bin/bash

build_dir="build"
executable_name="Daemon"

if command -v cmake &> /dev/null
then
    echo "CMake уже установлен. Версия:"
    cmake --version
else
    echo "CMake не установлен."
    exit 1
fi

# Проверка наличия пакета libconfig++-dev
if dpkg -l | grep -q libconfig++-dev
then
    echo "Пакет libconfig++-dev установлен."
else
    echo "Пакет libconfig++-dev не установлен."
    echo "Для установки выполните команду: sudo apt install libconfig++-dev"
    exit 1
fi

if [[ (-e $build_dir) && (-d $build_dir) ]]
then
    echo "Удаление старой директории сборки..."
    rm -r $BUILD_DIR
fi

echo "Создание директории сборки..."
mkdir $build_dir

cd $build_dir || exit

echo "Запуск cmake..."
cmake .. -DCMAKE_C_FLAGS="-Wall -Werror" -DCMAKE_CXX_FLAGS="-Wall -Werror"

echo "Запуск make..."
make

if [[ $? -eq 0 ]]
then
    echo "Сборка завершена успешно."
    cp $executable_name ../
    cd ..
    echo "Удаление директории сборки..."
    rm -r $build_dir
    echo "Запуск программы..."
    ./$executable_name
    rm $executable_name
else
    echo "Ошибка при сборке"
    exit 1
fi
