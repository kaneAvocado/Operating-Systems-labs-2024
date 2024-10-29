#include "daemon_utils.h"
#include <iostream>

void daemonize() {

    pid_t pid = fork();

    if (pid < 0) {
        exit(EXIT_FAILURE);  
    }

    if (pid > 0) {
        exit(EXIT_SUCCESS);  
    }


    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }
 
    signal(SIGCHLD, SIG_IGN);
 

    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    umask(0);

    if (chdir("/") < 0) {
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);


    open("/dev/null", O_RDONLY); // STDIN
    open("/dev/null", O_RDWR);   // STDOUT
    open("/dev/null", O_RDWR);   // STDERR

}