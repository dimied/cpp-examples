#!/bin/bash
#
reset

g++ -std=c++11 -c convert_base64.cpp ; g++ -std=c++11 -o test convert_base64.cpp example.cpp

g++ -std=c++11 -O3 -o testopt convert_base64.cpp example.cpp 

./test
