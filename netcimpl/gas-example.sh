#!/bin/bash
#
#

# strip gas-example.o
ls a.out >> /dev/null 2>&1 && rm -f a.out
ls gas-example.o >> /dev/null 2>&1 && rm -f gas-example.o
ls gas-example-as.o >> /dev/null 2>&1 && rm -f gas-example-as.o
ls gas-example >> /dev/null 2>&1 && rm -f gas-example
ls gas-example-min >> /dev/null 2>&1 && rm -f gas-example-min
ls gas-example-nasm >> /dev/null 2>&1 && rm -f gas-example-nasm
ls gas-example-nasm-min >> /dev/null 2>&1 && rm -f gas-example-nasm-min

#
nasm -f elf64 gas-example.asm
# nasm -f elfx32 gas-example.asm
r=$?
echo "NASM "$r
wc -c gas-example.o
# gcc -Wall -no-pie -fno-ident -nostartfiles -nostdlib -ffreestanding gas-example.o
ld -s gas-example.o
r=$?
echo "LD-NASM "$r

mv a.out gas-example-nasm
wc -c gas-example-nasm
# exit 0

as gas-example.s -o gas-example-as.o
r=$?
echo "AS "$r

# ld -o gas-example gas-example.o 
gcc -Wall -no-pie -s -nostdlib gas-example-as.o 
r=$?
echo "LD "$r
wc -c a.out
gcc -Wall -Os -no-pie -s -nostdlib gas-example.s 
# ld -s gas-example.o 
#gcc -Wall  -s gas-example.s
r=$?
echo "LD "$r
wc -c a.out
mv a.out gas-example

cp gas-example gas-example-min
strip -s gas-example-min

r=$?
echo "S "$r

# ls -lag gas-example*
wc -c gas-example.o
wc -c gas-example
wc -c gas-example-min
wc -c gas-example-nasm
cp gas-example-nasm gas-example-nasm-min
strip -s gas-example-nasm-min
wc -c gas-example-nasm-min
cp gas-example gas-example-2
gzexe gas-example-2
wc -c gas-example-2
./gas-example-2

./gas-example
