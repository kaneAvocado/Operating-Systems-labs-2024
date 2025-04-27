#include <QApplication>
#include "HostUtils/HostWindow.hpp"
#include "HostUtils/hostProcessing.hpp"
#include "ClientUtils/clientProcessing.hpp"
#include "conn_fifo.hpp"
#include "semaphore.hpp"
#include "logger.hpp"

int main(int argc, char* argv[]) {

    // initialize some stuff
    std::vector<Book> books = {
        {"Book 1", 10},
        {"Book 2", 5},
        {"Book 3", 20},
        {"Book 4", 0}
    };
    Semaphore semaphore(1);

    auto& logger = LoggerHost::get_instance();

    ConnFifo hostFifo("/tmp/my_fifo", logger);
    if (!hostFifo.IsInitialized()) {
        logger.log(Status::ERROR, "Failed to initialize host queue");
        return EXIT_FAILURE;
    }

    // Make child process
    pid_t pid = fork();
    if (pid == -1) {

        // Error
        logger.log(Status::ERROR, "Failed to fork client");
        return EXIT_FAILURE;

    } else if (pid == 0) {

        // this is child process -> start client
        ConnFifo clientFifo("/tmp/my_fifo", LoggerClient::get_instance());
        if (!clientFifo.IsInitialized()) {
            logger.log(Status::ERROR, "Failed to initialize client queue");
            return EXIT_FAILURE;
        }
        return processClient(semaphore, clientFifo, books);

    } else {

        // this is main process -> start host
        QApplication app(argc, argv);
        return processHost("Communication by queue", semaphore, hostFifo, books, app, pid);

    }
}
