#!/bin/bash

as compression/inflate_asm.s -o inflate_asm.o
r=$?

if [ $r -ne 0 ]; then
echo "Assembler failed"

exit 0

fi

#must be at the end: -lssl -lcrypto
gcc -Wall -Wextra -pedantic -fPIC -O3 compression/inflate_asm.s http.c -lssl -lcrypto

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