#!/bin/bash

nm -S --size-sort --radix=d ./a.out

nm -S --radix=d ./inflate_asm.o 