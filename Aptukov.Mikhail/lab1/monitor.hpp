#pragma once
#include <cerrno>
#include <poll.h>
#include <cstdio>
#include <cstdlib>
#include <sys/inotify.h>
#include <unistd.h>
#include <cstring>
#include <syslog.h>
#include <vector>
#include <string>

namespace monitor{
    int start_monitor(const std::vector<std::string> &path_list);
}