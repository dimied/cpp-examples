#!/bin/bash

#must be at the end: -lssl -lcrypto
gcc -Wall -Wextra -Wundef -pedantic -fPIC -ggdb -Wno-variadic-macros http.c -lssl -lcrypto

r=$?
echo "COMPILE-RES: "$r

echo "Params:"$1

if [ $r -eq 0 ]; then

wc -c ./a.out

echo ""

if [ "$1" = "tofile" ]; then

echo "To file ..."
# no buffering
stdbuf -o0 ./a.out | tee net_result.temp.txt

else
./a.out
fi

echo ""

wc -c ./a.out

fi



#echo "---DONE---"
#./a.out