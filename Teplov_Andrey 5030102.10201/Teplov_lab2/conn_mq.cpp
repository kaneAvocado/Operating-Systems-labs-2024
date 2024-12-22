#include "conn_mq.hpp"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>

ConnMq::ConnMq(key_t key, LoggerHost& logger) : queueKey(key), isHost(true)
{
    int flags = IPC_CREAT | 0666;
    queueId = msgget(key, flags);

    if (queueId == -1) {
        logger.log(Status::ERROR, "Failed to open message queue");
    }
}

ConnMq::ConnMq(key_t key, LoggerClient& logger) : queueKey(key), isHost(false)
{
    int flags = 0666;
    queueId = msgget(key, flags);

    if (queueId == -1) {
        logger.log(Status::ERROR, "Failed to open message queue");
    }
}

ConnMq::~ConnMq() {
    if (IsInitialized()) {
        msgctl(queueId, IPC_RMID, nullptr); // TODO: some check
    }
}

bool ConnMq::Read(void* buf, size_t count) {
    if (!IsInitialized()) {
        return false;
    }

    struct msgbuf {
        long mtype;
        char mtext[1024];
    } message;

    ssize_t bytesRead = msgrcv(queueId, &message, sizeof(message.mtext), !isHost + 1, 0); // !isHost: if host -> read client else read host
    if (bytesRead == -1) {
        return false;
    }

    std::memcpy(buf, message.mtext, std::min(count, static_cast<size_t>(bytesRead)));
    return true;
}

bool ConnMq::Write(const void* buf, size_t count) {
    if (!IsInitialized()) {
        return false;
    }

    struct msgbuf {
        long mtype;
        char mtext[1024];
    } message;

    message.mtype = isHost + 1; // if host -> write as host (+ 1 because mtype > 0)
    if (count > sizeof(message.mtext)) {
        return false;
    }

    std::memcpy(message.mtext, buf, count);
    if (msgsnd(queueId, &message, count, 0) == -1) {
        return false;
    }

    return true;
}

bool ConnMq::IsInitialized() const {
    return (queueId != -1);
}
