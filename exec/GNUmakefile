# Makefile for GMake (Linux/macOS)
.PHONY: all clean

all: clean execute

# strip

# -s : removes debug info
# -fno-math-errno : no errno available
# && gcc -S execute.c
execute: execute.c
	gcc -Os -s -o execute execute.c -lm && gcc -S execute.c

strip: execute
	strip --strip-unneeded -R .comment ./execute

stripall: execute
	strip --strip-all -R .comment ./execute

clean:
	rm -f execute
