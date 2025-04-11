all: main test wc tee

tee: main
	rm -f build/tee
	ln -s main build/tee

wc: main
	rm -f build/wc
	ln -s main build/wc

main:
	gcc -g -Wall scr/main.c scr/internal_utils.h -o build/main

test:
	gcc -g scr/test.c -o build/test