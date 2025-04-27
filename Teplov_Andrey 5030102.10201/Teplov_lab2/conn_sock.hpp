#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include "logger.hpp"
#include "conn.hpp"

class ConnSock : public conn {
public:
    ConnSock(int hostPort, LoggerHost& logger);
    ConnSock(int hostPort, LoggerClient& logger);
    ~ConnSock();

    ConnSock* Accept(LoggerHost& logger); // only for host

    bool Read(void* buf, size_t count);
    bool Write(const void* buf, size_t count);

    bool IsInitialized() const {
        return sock_fd != -1;
    }

private:
    int sock_fd;              // Socket file Descriptor
    struct sockaddr_in addr;  // socket address

    ConnSock() = default; // for accept
};
