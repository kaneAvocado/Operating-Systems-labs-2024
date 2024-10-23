**Что должно быть установлено**<br>
1. **bash**: sudo apt install bash
2. **cmake**: sudo apt install cmake
3. **make**: sudo install make
4. **g++**: sudo install g++
<br>
**Сборка проекта**<br>
1. Загрузить папку lab1
2. В терминале перейти в папку lab1: cd путь/до/папки/lab1
3. В папке lab1 в терминале запустить команду сборки: **bash build.sh**
<br>
**Запуск проекта**<br>
* В папке lab1 в терминале ввести команду: **\lab1_DAEMON config.conf**
  (здесь после обратного слеша пишется имя проекта, как оно прописано в CMakeLists.txt; второй параметр - это название конфигурационного файла)
<br>
**Команды применимые после запуска**<br>
1. Просмотр логов: **cat /var/log/syslog**
2. Вызов сигнала SIGHUP: **sudo kill -SIGHUP $(cat /var/run/lab1.pid)**
3. Вызов сигнала SIGTERM: **sudo kill -SIGTERM $(cat /var/run/lab1.pid)**
<br>
**Дополнительные команды**<br>
* Переход в папку **cd ./folder1**
* Создание файла **touch nameFile.type**
