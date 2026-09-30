#!/bin/bash

#must be at the end: -lssl -lcrypto
gcc -Wall -Wextra -pedantic -fPIC -O3 http.c -lssl -lcrypto

r=$?
echo "COMPILE-RES: "$r

if [ $r -eq 0 ]; then
wc -c ./a.out
echo ""


if [ "$1" = "tofile" ]; then

echo "To file ..."
# no buffering
stdbuf -o0 ./a.out | tee net_result_opt.temp.txt

else
./a.out
fi

echo ""
wc -c ./a.out

fi



#echo "---DONE---"
#./a.out