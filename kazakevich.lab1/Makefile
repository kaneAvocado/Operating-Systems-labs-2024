
CXX = g++

CXXFLAGS = -Wall -g -Werror

SRCS = ./main.cpp ./ConfigManager.cpp ./Daemon.cpp ./PidfileManager.cpp ./daemon_utils.cpp

TARGET = my_program

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^


clean:
	rm -f $(TARGET)
