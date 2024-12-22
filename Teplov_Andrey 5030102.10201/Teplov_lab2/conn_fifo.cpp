#include "conn_fifo.hpp"

ConnFifo::ConnFifo(const std::string& fifoPath, LoggerHost& logger) : path(fifoPath), readFd(-1), writeFd(-1) {

    if (mkfifo(path.c_str(), 0666) == -1) {
        if (errno != EEXIST) {
            logger.log(Status::ERROR, "Failed to create FIFO");
            return;
        }
    }

    readFd = open(path.c_str(), O_RDONLY | O_NONBLOCK); // open for read
    if (readFd == -1) {
        logger.log(Status::ERROR, "Failed to open FIFO for reading");
        return;
    }

    writeFd = open(path.c_str(), O_WRONLY); // open for write
    if (writeFd == -1) {
        logger.log(Status::ERROR, "Failed to open FIFO for writing");
        close(readFd);
        return;
    }
}

ConnFifo::ConnFifo(const std::string& fifoPath, LoggerClient& logger) : path(fifoPath), readFd(-1), writeFd(-1) {

    readFd = open(path.c_str(), O_RDONLY | O_NONBLOCK); // open for read
    if (readFd == -1) {
        logger.log(Status::ERROR, "Failed to open FIFO for reading");
        return;
    }

    writeFd = open(path.c_str(), O_WRONLY); // open for write
    if (writeFd == -1) {
        logger.log(Status::ERROR, "Failed to open FIFO for writing");
        close(readFd);
        return;
    }
}

bool ConnFifo::IsInitialized() const {
    return writeFd != -1 && readFd != -1;
}

ConnFifo::~ConnFifo() {
    if (readFd != -1) {
        close(readFd);
    }
    if (writeFd != -1) {
        close(writeFd);
    }

    if (!path.empty()) {
        unlink(path.c_str());
    }
}

bool ConnFifo::Read(void* buf, size_t count) {
    if (!IsInitialized()) {
        return false;
    }

    ssize_t bytesRead = read(readFd, buf, count);
    if (bytesRead == -1) {
        return false;
    }

    return bytesRead > 0;
}

bool ConnFifo::Write(const void* buf, size_t count) {
    if (!IsInitialized()) {
        return false;
    }

    ssize_t bytesWritten = write(writeFd, buf, count);
    if (bytesWritten == -1) {
        return false;
    }

    return static_cast<size_t>(bytesWritten) == count;
}
