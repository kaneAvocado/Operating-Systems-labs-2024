
CXX = g++

CXXFLAGS = -Wall -g -Werror -std=c++17

SRCS = ./main.cpp ./Daemon.cpp 

TARGET = daemonLab

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^


clean:
	rm -f $(TARGET)
