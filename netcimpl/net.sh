#!/bin/bash

cp compression/inflate_asm.asm compression/inflate_asm.s
as --64 compression/inflate_asm.s -o inflate_asm.o
r=$?

if [ $r -ne 0 ]; then
echo "Assembler failed"

exit 0

fi

#
#must be at the end: -lssl -lcrypto

gcc -Wall -Wextra -Wundef -pedantic -fPIE -ggdb -Wno-variadic-macros ./compression/inflate_asm.s http.c -lssl -lcrypto
# gcc -Wall -Wextra -Wundef -pedantic -fPIE -ggdb -Wno-variadic-macros http.c -lssl -lcrypto

r=$?
echo "COMPILE-RES: "$r

#echo "Params:"$1

if [ $r -eq 0 ]; then

#gcc http.o inflate_asm.o -o a.out -lssl -lcrypto

wc -c ./a.out

echo ""

ulimit -c unlimited

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