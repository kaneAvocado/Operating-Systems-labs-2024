#include "conn_sock.hpp"
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>

ConnSock::ConnSock(int hostPort, LoggerHost& logger) : sock_fd(-1) {
    logger.log(Status::INFO, "Creating socket for Host");
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        logger.log(Status::ERROR, "Socket creation failed");
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(hostPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        logger.log(Status::ERROR, "Bind failed");
        close(sock_fd);
        sock_fd = -1;
        return;
    }

    if (listen(sock_fd, 1) == -1) {
        logger.log(Status::ERROR, "Listen failed");
        close(sock_fd);
        sock_fd = -1;
        return;
    }

    logger.log(Status::INFO, "Socket successfully created and listening");
}

ConnSock::ConnSock(int hostPort, LoggerClient& logger) : sock_fd(-1) {
    logger.log(Status::INFO, "Creating socket for Client");
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        logger.log(Status::ERROR, "Socket creation failed");
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(hostPort);

    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        logger.log(Status::ERROR, "Invalid host IP address");
        close(sock_fd);
        sock_fd = -1;
        return;
    }

    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        logger.log(Status::ERROR, "Connect failed");
        close(sock_fd);
        sock_fd = -1;
        return;
    }

    logger.log(Status::INFO, "Connected to host successfully");
}

ConnSock* ConnSock::Accept(LoggerHost& logger) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd == -1) {
        logger.log(Status::ERROR, "Accept failed");
        return nullptr;
    }

    logger.log(Status::INFO, "Client connection accepted");
    // make new socket for communication with client
    ConnSock* conn = new ConnSock();
    conn->sock_fd = client_fd;
    conn->addr = client_addr;
    return conn;
}

bool ConnSock::Read(void* buf, size_t count) {
    if (!IsInitialized()) {
        return false;
    }

    ssize_t bytes_read = recv(sock_fd, buf, count, 0);
    if (bytes_read <= 0) {
        return false;
    }
    return true;
}

bool ConnSock::Write(const void* buf, size_t count) {
    if (!IsInitialized()) {
        return false;
    }

    ssize_t bytes_sent = send(sock_fd, buf, count, 0);
    if (bytes_sent <= 0) {
        return false;
    }
    return true;
}

ConnSock::~ConnSock() {
    if (sock_fd != -1) {
        close(sock_fd);
    }
}
