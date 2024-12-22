#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "conn.hpp"
#include "logger.hpp"

class ConnFifo : public conn {
public:
    ConnFifo(const std::string& fifoPath, LoggerHost& logger);
    ConnFifo(const std::string& fifoPath, LoggerClient& logger);
    ~ConnFifo() override;

    bool Read(void* buf, size_t count) override;
    bool Write(const void* buf, size_t count) override;

    bool IsInitialized() const override;

private:
    std::string path;
    int readFd;
    int writeFd;
};
