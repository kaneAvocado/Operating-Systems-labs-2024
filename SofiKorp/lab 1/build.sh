# Функция для вывода сообщений об ошибках и завершения скрипта
function abort {
    echo "Ошибка: $1" >&2
    exit 1
}

# Удаление предыдущих сборок через Makefile
make clean || abort "Не удалось очистить предыдущую сборку."

# Сборка проекта с использованием Makefile
make daemonLab || abort "Не удалось собрать проект."

echo "Сборка выполнена успешно."

# Проверка наличия конфигурационного файла и директории для PID-файла
CONFIG_FILE="./config.txt"
PID_DIR=$(dirname "$CONFIG_FILE")

if [[ ! -f $CONFIG_FILE ]]; then
    abort "Конфигурационный файл отсутствует: $CONFIG_FILE"
fi

if [[ ! -d $PID_DIR ]]; then
    abort "Директория для PID-файла отсутствует: $PID_DIR"
fi

# Запуск демона
echo "Запускаем демона..."
sudo ./daemonLab "$CONFIG_FILE" "$PID_DIR/daemon.pid" || abort "Не удалось запустить демона."

echo "Демон успешно запущен."

# Запрос на удаление исполняемого файла
read -p "Хотите удалить исполняемый файл? (y/n): " response
if [[ $response == "y" ]]; then
    sudo rm -f ./daemonLab || abort "Не удалось удалить исполняемый файл."
    echo "Исполняемый файл удалён."
else
    echo "Исполняемый файл остался."
fi