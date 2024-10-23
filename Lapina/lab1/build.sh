#!/bin/bash

PID_PATH="/var/run/lab1.pid"

# create pid_file
[[ -f PID_PATH ]] || sudo touch "$PID_PATH"

sudo chmod 666 "$PID_PATH"

cmake .
make
rm CMakeCache.txt Makefile cmake_install.cmake

FILE=lab1.cbp

if [[ -f "$FILE" ]]; then
    rm "$FILE"
fi

rm -r CMakeFiles/
