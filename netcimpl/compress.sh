#!/bin/bash
#
#
cp compression/inflate_asm.asm compression/inflate_asm.s
as --64 compression/inflate_asm.s -o inflate_asm.o
r=$?

if [ $r -ne 0 ]; then
echo "Assembler failed"

exit 0

fi

gcc -Wall -Wextra -Wundef -pedantic -fPIE -ggdb -Wno-variadic-macros compress.c ./compression/inflate_asm.s
# gcc -Wall -Wextra -Wundef -pedantic -fPIE -ggdb -Wno-variadic-macros -c compress.c
r=$?
echo "COMPILE-RES: "$r

# ld --entry=main -o ./a.out compress.o inflate_asm.o -lc
# r=$?
# echo "LD-RES: "$r

#echo "Params:"$1

OUTFILE=a.out

if [ -f "$OUTFILE" ]; then
    echo "$OUTFILE exists."
    #echo ""
else
    echo "$OUTFILE does not exist."
fi

if [ $r -eq 0 ]; then

wc -c a.out

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

wc -c a.out

if [ "$1" = "tofile" ]; then
echo "Output: net_result.temp.txt"
fi

fi



#echo "---DONE---"
#./a.out