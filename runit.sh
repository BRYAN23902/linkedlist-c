#!/bin/bash

# shortcut to compile and run the program

rm -f main
gcc -o main -g -Wall -Wextra -Wno-sign-compare *.c
./main

