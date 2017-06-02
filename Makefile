all:
	gcc -g -c src/json.c -Wall -Werror -fpic -o json.o
	gcc -shared -o mods/json/libctrjson.so json.o -ljson-c

clean:
	rm mods/json/*

