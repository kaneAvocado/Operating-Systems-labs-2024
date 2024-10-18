#include "daemon_utils.h"
#include <iostream>

void daemonize() {

    pid_t pid = fork();

    if (pid < 0) {
        //std::cerr << "Error fork pid < 0" << std::endl;
        exit(EXIT_FAILURE);  // Ошибка fork
    }

    if (pid > 0) {
        //std::cerr << "End parent process" << std::endl;
        exit(EXIT_SUCCESS);  // Завершаем родительский процесс
    }

    //std::cerr << "Create new s" << std::endl;
    // Создаем новый сеанс
    if (setsid() < 0) {
        std::cerr << "setsid() < 0 error" << std::endl;
        exit(EXIT_FAILURE);
    }
    //std::cerr << "ignore signals" << std::endl;
    // Игнорируем сигналы
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

   // Второй fork для предотвращения возможности открепления от терминала
    pid = fork();
    if (pid < 0) {
        //std::cerr << "second fork error pid < 0" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        //std::cerr << "second fork error pid > 0" << std::endl;
        exit(EXIT_SUCCESS);
    }

    //std::cerr << "Set umask" << std::endl;
    // Устанавливаем umask и рабочую директорию
    umask(0);

    //std::cerr << "chdir" << std::endl;
    if (chdir("/") < 0) {
        //std::cerr << "chdir error" << std::endl;
        exit(EXIT_FAILURE);
    }

    //std::cerr << "close files" << std::endl;
    // Закрываем стандартные файловые дескрипторы
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);

    //std::cerr << "open" << std::endl;
    // Перенаправляем стандартные дескрипторы на /dev/null
    //open("/dev/null", O_RDONLY); // STDIN
    //open("/dev/null", O_RDWR);   // STDOUT
    //open("/dev/null", O_RDWR);   // STDERR
    //std::cerr << "end demonize" << std::endl;
}