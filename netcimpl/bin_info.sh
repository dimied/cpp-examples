#!/bin/bash

nm -S --size-sort --radix=d ./a.out

nm -S --radix=d ./inflate_asm.o

nm -S --radix=d ./other_asm.o