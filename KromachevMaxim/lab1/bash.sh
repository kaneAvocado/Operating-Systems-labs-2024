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

if [ -e $build_dir ] && [ -d $build_dir ]
then
    echo "Удаление старой директории сборки..."
    rm -r $build_dir
fi

if [ -e $executable_name ]
then
    rm $executable_name
fi

echo "Создание директории сборки..."
mkdir $build_dir

cd $build_dir || exit

echo "Запуск cmake..."
cmake .. -DCMAKE_C_FLAGS="-Wall -Werror" -DCMAKE_CXX_FLAGS="-Wall -Werror"

echo "Запуск make..."
make

if [ $? -eq 0 ]
then
    echo "Сборка завершена успешно."
    cp $executable_name ../

    # Переход в родительскую директорию
    cd .. || { echo "Не удалось перейти в родительскую директорию"; exit 1; }

    echo "Удаление директории сборки..."
    rm -r $build_dir || { echo "Ошибка при удалении директории"; exit 1; }

    echo "Запуск программы..."
    ./$executable_name || { echo "Ошибка при запуске программы"; exit 1; }
else
    echo "Ошибка при сборке"
    exit 1
fi
