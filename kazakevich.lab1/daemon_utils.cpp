#include "daemon_utils.h"
#include <iostream>

void daemonize() {

    pid_t pid = fork();

    if (pid < 0) {
        //std::cerr << "Error fork pid < 0" << std::endl;
        exit(EXIT_FAILURE);  // Îøèáêà fork
    }

    if (pid > 0) {
        //std::cerr << "End parent process" << std::endl;
        exit(EXIT_SUCCESS);  // Çàâåðøàåì ðîäèòåëüñêèé ïðîöåññ
    }

    //std::cerr << "Create new s" << std::endl;
    // Ñîçäàåì íîâûé ñåàíñ
    if (setsid() < 0) {
        std::cerr << "setsid() < 0 error" << std::endl;
        exit(EXIT_FAILURE);
    }
    //std::cerr << "ignore signals" << std::endl;
    // Èãíîðèðóåì ñèãíàëû
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

   // Âòîðîé fork äëÿ ïðåäîòâðàùåíèÿ âîçìîæíîñòè îòêðåïëåíèÿ îò òåðìèíàëà
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
    // Óñòàíàâëèâàåì umask è ðàáî÷óþ äèðåêòîðèþ
    umask(0);

    //std::cerr << "chdir" << std::endl;
    if (chdir("/") < 0) {
        //std::cerr << "chdir error" << std::endl;
        exit(EXIT_FAILURE);
    }

    //std::cerr << "close files" << std::endl;
    // Çàêðûâàåì ñòàíäàðòíûå ôàéëîâûå äåñêðèïòîðû
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    //std::cerr << "open" << std::endl;
    // Ïåðåíàïðàâëÿåì ñòàíäàðòíûå äåñêðèïòîðû íà /dev/null
    open("/dev/null", O_RDONLY); // STDIN
    open("/dev/null", O_RDWR);   // STDOUT
    open("/dev/null", O_RDWR);   // STDERR
    //std::cerr << "end demonize" << std::endl;
}
