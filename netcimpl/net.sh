#!/bin/bash


as compression/inflate_asm.s -o inflate_asm.o
r=$?

if [ $r -ne 0 ]; then
echo "Assembler failed"

exit 0

fi

#
#must be at the end: -lssl -lcrypto
gcc -Wall -Wextra -Wundef -pedantic -ggdb -Wno-variadic-macros http.c ./compression/inflate_asm.s   -lssl -lcrypto

r=$?
echo "COMPILE-RES: "$r

#echo "Params:"$1

if [ $r -eq 0 ]; then

#gcc http.o inflate_asm.o -o a.out -lssl -lcrypto

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

if [ "$1" = "tofile" ]; then
echo "Output: net_result.temp.txt"
fi

fi



#echo "---DONE---"
#./a.out