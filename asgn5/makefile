libmsocket.a: mysocket.c mysocket.h
	gcc -Wall -c mysocket.c
	ar rcs libmsocket.a mysocket.o
	gcc server.c -o ser -lmsocket
	gcc client.c -o cli -lmsocket
	
install: libmsocket.a
	sudo cp mysocket.h /usr/local/include 
	sudo cp mysocket.h /usr/include
	sudo cp libmsocket.a /usr/lib

clean:
	rm libmsocket.a
	rm mysocket.o
	rm ser
	rm cli