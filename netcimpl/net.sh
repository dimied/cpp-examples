#!/bin/bash

gcc -Wall -ggdb http.c

r=$?
echo "COMPILE-RES: "$r

if [ $r -eq 0 ]; then

./a.out

wc -c ./a.out

fi



#echo "---DONE---"
#./a.out