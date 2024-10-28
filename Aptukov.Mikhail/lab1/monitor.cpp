#include "monitor.hpp"

static void handle_events(int fd, const std::vector<int>& wd, const std::vector<std::string>& path_list)
{
    char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;

    for (;;)
    {
        len = read(fd, buf, sizeof(buf));

        if (len == -1 && errno != EAGAIN)
        {
            syslog(LOG_ERR, "read failed");
            return;
        }

        if (len <= 0)
            break;

        for (char *ptr = buf; ptr < buf + len; ptr += sizeof(struct inotify_event) + event->len)
        {
            event = reinterpret_cast<const struct inotify_event*>(ptr);

            std::string message;

            if (event->mask & IN_OPEN)
                message += "IN_OPEN: ";
            if (event->mask & IN_CLOSE_NOWRITE)
                message += "IN_CLOSE_NOWRITE: ";
            if (event->mask & IN_CLOSE_WRITE)
                message += "IN_CLOSE_WRITE: ";

            for (size_t i = 0; i < wd.size(); ++i)
            {
                if (wd[i] == event->wd)
                {
                    message += path_list[i] + "/";
                    break;
                }
            }

            if (event->len)
                message += event->name;

            if (event->mask & IN_ISDIR)
                message += " [directory]";
            else
                message += " [file]";

            syslog(LOG_INFO, "%s", message.c_str());
        }
    }
}

int monitor::start_monitor(const std::vector<std::string> &path_list)
{
    struct pollfd fds[1];
    nfds_t nfds;
    int poll_num;
    int fd;
    std::vector<int> wd;

    syslog(LOG_DEBUG, "start_monitor: count = %lu", path_list.size());

    fd = inotify_init1(IN_NONBLOCK);

    if (fd == -1)
    {
        syslog(LOG_ERR, "inotify_init1 failed");
        return -1;
    }

    wd.resize(path_list.size());

    for (size_t i = 0; i < path_list.size(); i++)
    {
        syslog(LOG_DEBUG, "start_monitor: inotify_add_watch for %s", path_list[i].c_str());
        wd[i] = inotify_add_watch(fd, path_list[i].c_str(), IN_OPEN | IN_CLOSE);

        if (wd[i] == -1)
        {
            syslog(LOG_ERR, "Cannot watch '%s': %s", path_list[i].c_str(), strerror(errno));
            return -1;
        }
    }

    nfds = 1;
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while (true)
    {
        poll_num = poll(fds, nfds, -1);
        syslog(LOG_DEBUG, "start_monitor: poll_num = %d", poll_num);

        if (poll_num == -1)
        {
            if (errno == EINTR)
                continue;

            syslog(LOG_ERR, "poll failed");
            return -1;
        }

        if (poll_num > 0)
        {
            if (fds[0].revents & POLLIN)
            {
                handle_events(fd, wd, path_list);
            }
        }
    }

    close(fd);
    return 0;
}
