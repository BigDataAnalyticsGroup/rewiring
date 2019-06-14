#!/bin/bash

gcc -c -Wall -pedantic -std=gnu99 -g -D ERRORCHECK -mavx *.c tests/*.c methods/*.c -D_GNU_SOURCE;
g++ -c -std=c++11 -g cpp/*.cpp;
g++ -o Rewiring_Microbench_Debug *.o -lm -lrt -lpthread;
rm -rf *.o;
