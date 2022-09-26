# temporary makefile
all:
	$(CC) -DVERBOSE -Isrc/ src/libusb.c test.c src/packet.c src/operations.c src/deviceinfo.c -lusb -o test.o
	./test.o

clean:
	$(RM) *.o
