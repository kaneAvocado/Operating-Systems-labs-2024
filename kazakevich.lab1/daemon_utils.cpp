#include "daemon_utils.h"

void daemonize() {
    pid_t pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);  // Ошибка fork
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);  // Завершаем родительский процесс
    }

    // Создаем новый сеанс
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // Игнорируем сигналы
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // Второй fork для предотвращения возможности открепления от терминала
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Устанавливаем umask и рабочую директорию
    umask(0);
    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    // Закрываем стандартные файловые дескрипторы
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Перенаправляем стандартные дескрипторы на /dev/null
    open("/dev/null", O_RDONLY); // STDIN
    open("/dev/null", O_RDWR);   // STDOUT
    open("/dev/null", O_RDWR);   // STDERR
}