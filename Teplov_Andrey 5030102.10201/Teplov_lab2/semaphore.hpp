#pragma once

#include <semaphore.h>

class Semaphore {
public:
    Semaphore(unsigned int value);
    bool Wait();
    bool Post();
    ~Semaphore();

private:
    sem_t semaphore;
};
