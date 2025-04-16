all: main test wc tee ping

windows:
	x86_64-w64-mingw32-gcc -Wall scr/main.c -o build/main.exe

tee: main
	rm -f build/tee
	ln -s main build/tee

wc: main
	rm -f build/wc
	ln -s main build/wc

ping: main
	rm -f build/ping
	ln -s main build/ping

main:
	gcc -g -Wall scr/main.c scr/internal_utils.h -o build/main

test:
	gcc -g scr/test.c -o build/test