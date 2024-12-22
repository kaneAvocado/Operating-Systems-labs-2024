#include <QApplication>
#include "ClientWindow.hpp"
#include "conn_sock.hpp"
#include "logger.hpp"
#include "semaphore.hpp"
#include <sstream>
#include <thread>
#include <signal.h>

void listenForHostMessages(conn& conn, Semaphore& semaphore, ClientWindow& window, std::atomic<bool>& is_ranning) {
    auto& logger = LoggerClient::get_instance();

    while (is_ranning) {

        semaphore.Wait(); // Start critical section
        char buffer[64] = {0};
        if (conn.Read(buffer, sizeof(buffer))) {
            std::string response(buffer);

            if (response == "YES") {
                window.onSuccessTakeBook();
                logger.log(Status::INFO, "Host gives the book");
            } 
            else if (response == "NO") {
                window.onFailedTakeBook();
                logger.log(Status::INFO, "Host rejected the request: no such book");
            } 
            else {
                logger.log(Status::ERROR, "Unexpected response from host: " + response);
            }
        }

        semaphore.Post(); // End critical section
        sleep(0.01);
    }
}

int processClient(Semaphore& semaphore, conn& conn, const std::vector<Book>& books) {
    auto& logger = LoggerClient::get_instance();

    int argc = 0;
    char** argv = nullptr;
    QApplication app(argc, argv);
    ClientWindow window(books);

    std::atomic<bool> is_running(true);

    // Start thread with listenning socket
    std::thread listenerThread(listenForHostMessages, std::ref(conn), std::ref(semaphore), std::ref(window), std::ref(is_running));

    QObject::connect(&window, &ClientWindow::bookSelected, [&semaphore, &conn, &logger](const QString& bookName) {
        std::string request = "TAKE " + bookName.toStdString();
        if (conn.Write(request.c_str(), request.size())) {
            logger.log(Status::INFO, "Requested book: " + bookName.toStdString());
        }
        else {
            logger.log(Status::ERROR, "Failed to request the book: " + bookName.toStdString());
        }
    });

    QObject::connect(&window, &ClientWindow::bookReturned, [&conn, &logger](const QString& bookName) {
        std::string request = "RETURN " + bookName.toStdString();
        if (conn.Write(request.c_str(), request.size())) {
            logger.log(Status::INFO, "Returned book: " + bookName.toStdString());
        }
        else {
            logger.log(Status::ERROR, "Failed to return the book: " + bookName.toStdString());
        }
    });

    window.show();
    int result = app.exec();

    // Complete thread with listenning socket
    is_running = false;
    if (listenerThread.joinable()) {
        listenerThread.join();
    }

    return result;
}