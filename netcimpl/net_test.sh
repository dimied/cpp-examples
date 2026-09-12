#!/bin/bash

#must be at the end: -lssl -lcrypto
gcc -Wall -Wextra -pedantic -fPIC -ggdb -DTESTING=1 http.c -lssl -lcrypto

r=$?
echo "COMPILE-RES: "$r

if [ $r -eq 0 ]; then
wc -c ./a.out
echo ""
./a.out

echo ""
wc -c ./a.out

fi



#echo "---DONE---"
#./a.out