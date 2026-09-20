#!/bin/bash

#must be at the end: -lssl -lcrypto
gcc -Wall -Wextra -pedantic -fPIC -O3 http.c -lssl -lcrypto

r=$?
echo "COMPILE-RES: "$r

if [ $r -eq 0 ]; then
wc -c ./a.out

fi
