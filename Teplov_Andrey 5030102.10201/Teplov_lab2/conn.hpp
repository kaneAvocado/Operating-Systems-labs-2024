#pragma once

class conn {
public:
    virtual ~conn() {};

    virtual bool Read(void* buf, size_t count) = 0;
    virtual bool Write(const void* buf, size_t count) = 0;

    virtual bool IsInitialized() const = 0;
};
