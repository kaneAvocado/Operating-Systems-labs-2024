// include/utils.hpp
#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/types.h>  // Для определения pid_t

void daemonize();
bool processExists(pid_t pid);

#endif // UTILS_HPP
