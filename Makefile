# temporary makefile
all:
	$(CC) test.c -o test.o
	./test.o	

clean:
	$(RM) *.o
