#!/bin/bash

#must be at the end: -lssl -lcrypto
gcc -Wall -Wextra -Wundef -pedantic -fPIC -ggdb -Wno-variadic-macros -o ./net_codegen ./codegen/http_codegen.c

r=$?
echo "COMPILE-RES: "$r

if [ $r -eq 0 ]; then
wc -c ./a.out
echo ""
./net_codegen

echo ""
wc -c ./net_codegen

fi



#echo "---DONE---"
#./a.out