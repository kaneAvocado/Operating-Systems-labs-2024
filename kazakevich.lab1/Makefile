all: my_program

my_program: ./main.cpp ./ConfigManager.cpp ./Daemon.cpp ./PidfileController.cpp ./daemon_utils.cpp
    g++ -Wall -Werror -std=c++17 -o my_program ./main.cpp ./ConfigManager.cpp ./Daemon.cpp ./PidfileController.cpp ./daemon_utils.cpp


.PHONY: clean
clean:
	rm -f my_program
