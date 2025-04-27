#include "semaphore.hpp"
#include "logger.hpp"
#include <iostream>
#include <cstring>

Semaphore::Semaphore(unsigned int value) {
    if (sem_init(&semaphore, 0, value) == -1) {
        LoggerHost::get_instance().log(Status::ERROR, "Semaphore initialization failed");
    }
}

bool Semaphore::Wait() {
    if (sem_wait(&semaphore) == -1) {
        LoggerHost::get_instance().log(Status::ERROR, "Semaphore wait failed");
        return false;
    }
    return true;
}

bool Semaphore::Post() {
    if (sem_post(&semaphore) == -1) {
        LoggerHost::get_instance().log(Status::ERROR, "Semaphore post failed");
        return false;
    }
    return true;
}

Semaphore::~Semaphore() {
    if (sem_destroy(&semaphore) == -1) {
        LoggerHost::get_instance().log(Status::ERROR, "Semaphore destruction failed");
    }
}
