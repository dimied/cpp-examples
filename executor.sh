#!/bin/bash


gcc -Wall -Wextra -O3 executor.c 

#-t d for decimal
nm -S --size-sort -t d a.out | grep execute

./a.out