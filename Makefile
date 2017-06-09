all:
	gcc -g -c src/json.c -Wall -Werror -fpic -o json.o
	gcc -shared -o mods/json/libctrjson.so json.o -ljson-c
install:
	sh runtests.sh
clean:
	rm mods/json/*

