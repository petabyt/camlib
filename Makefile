# temporary makefile
all:
	$(CC) -Isrc/ test.c src/packet.c -o test.o
	./test.o	

clean:
	$(RM) *.o
