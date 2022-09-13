# temporary makefile
all:
	$(CC) -Isrc/ src/libusb.c test.c src/packet.c -lusb -o test.o
	./test.o	

clean:
	$(RM) *.o
