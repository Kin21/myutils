all: main test

main:
	gcc -g -Wall scr/main.c scr/internal_utils.h -o build/main

test:
	gcc -g scr/test.c -o build/test