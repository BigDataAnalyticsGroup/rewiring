#!/bin/bash

gcc -c -Wall -pedantic -std=gnu99 -O3 -mavx *.c methods/*.c tests/*.c -D_GNU_SOURCE;
g++ -c -std=c++11 -O3 cpp/*.cpp;
g++ -o Rewiring_Microbench *.o -lm -lrt -lpthread;
rm -rf *.o;
