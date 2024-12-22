#include <QApplication>
#include "HostUtils/HostWindow.hpp"
#include "HostUtils/hostProcessing.hpp"
#include "ClientUtils/clientProcessing.hpp"
#include "conn_sock.hpp"
#include "semaphore.hpp"
#include "logger.hpp"
#include <unistd.h>
#include <csignal>

int main(int argc, char* argv[]) {

    // take port
    if (argc != 2) {
        std::cerr << "Usage: ./host <port>" << std::endl;
        return EXIT_FAILURE;
    }

    // initialize some stuff
    int port = std::stoi(argv[1]);
    std::vector<Book> books = {
        {"Book 1", 10},
        {"Book 2", 5},
        {"Book 3", 20},
        {"Book 4", 0}
    };
    Semaphore semaphore(1);

    auto& logger = LoggerHost::get_instance();

    // Create hostSocket for incomming connection requests from client
    ConnSock hostSocket(port, logger);
    if (!hostSocket.IsInitialized()) {
        logger.log(Status::ERROR, "Failed to initialize host socket");
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
        ConnSock clientSocket(port, LoggerClient::get_instance());
        if (!clientSocket.IsInitialized()) {
            logger.log(Status::ERROR, "Failed to initialize client socket");
            return EXIT_FAILURE;
        }
        return processClient(semaphore, clientSocket, books);

    } else {

        // this is main process -> start host
        ConnSock* conn = hostSocket.Accept(logger);
        if (!conn)
        {
            logger.log(Status::ERROR, "Failed to accept connection");
            return EXIT_FAILURE;
        }

        QApplication app(argc, argv);
        int res = processHost("Host Port: " + std::string(argv[1]), semaphore, *conn, books, app, pid);
        delete conn;
        return res;

    }
}
