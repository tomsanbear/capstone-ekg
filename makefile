CC=gcc

install: ekgdatacollection.o
	$(CC) -I. -o main ekgdatacollection.o -lusb -lGoIO
	chmod u+x main

clean: 
	rm -f *.o main
