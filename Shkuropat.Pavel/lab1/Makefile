all: daemon

daemon: ./src/utils.cpp ./src/pidfile.cpp ./src/daemon.cpp ./src/main.cpp
	g++ -Wall -Werror -std=c++17 -o daemon ./src/utils.cpp ./src/pidfile.cpp ./src/daemon.cpp ./src/main.cpp

.PHONY: clean
clean:
	rm -f daemon