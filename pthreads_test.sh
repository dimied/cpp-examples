#!/bin/bash

#-L /urs/lib -L /usr/lib/x86_64-linux-gnu/
#-print-search-dirs
#-static 
#-fPIC
gcc -Wall -Wextra -pedantic -fPIC -O3  pthreads.c -lpthread  -o pthreads_test

r=$?
echo "COMPILE-RES: "$r

if [ $r -eq 0 ]; then

./pthreads_test

wc -c ./pthreads_test

fi