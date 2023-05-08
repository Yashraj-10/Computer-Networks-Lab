libmsocket.a: mysocket.c mysocket.h
	gcc -Wall -c mysocket.c
	ar rcs libmsocket.a mysocket.o
	
install: libmsocket.a
	sudo cp mysocket.h /usr/local/include 
	sudo cp mysocket.h /usr/include
	sudo cp libmsocket.a /usr/lib
