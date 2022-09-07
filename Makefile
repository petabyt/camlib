# temporary makefile
all:
	$(CC) -Isrc/ test.c src/piclib.c -o test.o
	./test.o	

clean:
	$(RM) *.o
