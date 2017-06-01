all:
	gcc -c src/jsonobject.c -Wall -Werror -fpic -o jsonobject.o
	gcc -shared -o mods/jsonobject/libctrjsonobject.so jsonobject.o -ljson-c

clean:
	rm mods/jsonobject/*

