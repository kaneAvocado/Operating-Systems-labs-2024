#!/bin/bash

# Функция для вывода сообщения об ошибке и завершения скрипта
error_exit() {
    echo "$1" 1>&2
    exit 1
}

# Очистка предыдущей сборки через Makefile
make clean || error_exit "Ошибка очистки предыдущей сборки"

# Сборка проекта с использованием Makefile
make daemon || error_exit "Ошибка сборки проекта"

echo "Сборка завершена успешно."

# Проверка наличия конфигурационного файла и директории для PID-файла
CONFIG_FILE="./test/config.txt"
PID_FILE="./test/daemon.pid"

if [[ ! -f $CONFIG_FILE ]]; then
    error_exit "Файл конфигурации не найден: $CONFIG_FILE"
fi

if [[ ! -d $(dirname "$PID_FILE") ]]; then
    error_exit "Директория для PID-файла не найдена: $(dirname "$PID_FILE")"
fi

# Запуск демона
echo "Запуск демона..."
sudo ./daemon "$CONFIG_FILE" "$PID_FILE" || error_exit "Ошибка запуска демона"

# Вывод сообщения об успешном запуске
echo "Демон запущен успешно."

# Удаление исполняемого файла (если нужно)
read -p "Хотите удалить исполняемый файл (y/n)? " choice
if [[ $choice == "y" ]]; then
    sudo rm -f ./daemon || error_exit "Ошибка удаления исполняемого файла"
    echo "Исполняемый файл удален."
else
    echo "Исполняемый файл не был удален."
fi
