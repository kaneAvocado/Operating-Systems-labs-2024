#pragma once

#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include "conn.hpp"
#include "logger.hpp"

class ConnMq : public conn {
public:
    ConnMq(key_t key, LoggerHost& logger);
    ConnMq(key_t key, LoggerClient& logger);
    ~ConnMq() override;

    bool Read(void* buf, size_t count) override;
    bool Write(const void* buf, size_t count) override;

    bool IsInitialized() const override;

private:
    key_t queueKey;
    int queueId;
    bool isHost; // for read/write msg type
};
