PTP_FILES=src/libusb.o src/packet.o src/operations.o src/deviceinfo.o

CFLAGS=-c -D VERBOSE -Isrc/

all: test.o
	./test.o

test.o: $(PTP_FILES) main.o src/*.h *.c
	$(CC) $(PTP_FILES) main.o -lusb -o test.o

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) *.o src/*.o
