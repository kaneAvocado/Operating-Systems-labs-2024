all: my_daemon

my_daemon: monitor.cpp my_daemon.cpp main.cpp
	g++ -Wall -Werror -std=c++17 -o my_daemon monitor.cpp my_daemon.cpp main.cpp

.PHONY: clean
clean:
	rm -f my_daemon
