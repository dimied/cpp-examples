#!/bin/bash

#-L /urs/lib -L /usr/lib/x86_64-linux-gnu/
#-print-search-dirs
#-static 
#-fPIC
gcc -Wall -Wextra -pedantic -fPIC -O3  linux_dltest.c -lc -ldl -lGL  -o linux_dltest

r=$?
echo "COMPILE-RES: "$r

if [ $r -eq 0 ]; then

./linux_dltest

wc -c ./linux_dltest

fi