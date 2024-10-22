#include <iostream>
#include "daemon.h"

int main(void) {

    Daemon* daemonPtr = Daemon::InstancePtr();
    daemonPtr->Start();
    daemonPtr->ReadConfig();
    daemonPtr->Proccessing();
    exit(EXIT_SUCCESS);
}

