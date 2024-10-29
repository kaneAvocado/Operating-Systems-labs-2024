
# Функция для завершения скрипта с сообщением об ошибке
function exit_with_error {
    echo "Ошибка: $1" >&2
    exit 1
}

# Очистка предыдущих сборок
make clean || exit_with_error "Не удалось очистить предыдущую сборку."

# Сборка проекта
make my_program || exit_with_error "Не удалось собрать проект."

echo "Сборка завершена успешно."

CONFIG_FILE="./test/config.txt"

if [[ ! -f $CONFIG_FILE ]]; then
    exit_with_error "Конфигурационный файл отсутствует: $CONFIG_FILE"
fi

# Запуск программы
echo "Запускаем программу..."
sudo ./my_program "$CONFIG_FILE" || exit_with_error "Не удалось запустить программу."

echo "Программа успешно запущена."

# Запрос на удаление исполняемого файла
read -p "Хотите удалить исполняемый файл? (y/n): " answer
if [[ $answer == "y" ]]; then
    sudo rm -f ./my_program || exit_with_error "Не удалось удалить исполняемый файл."
    echo "Исполняемый файл удалён."
else
    echo "Исполняемый файл оставлен."
fi
