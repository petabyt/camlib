-include config.mak

CD?=cd
PYTHON3?=python3
FILES=src/libusb.o src/operations.o src/packet.o src/deviceinfo.o src/enum.o

CFLAGS=-lusb -Isrc/

%.o: %.c src/*.h
	$(CC) -c $(CFLAGS) $< -o $@

src/enum.o: src/ptp.h src/canon.h src/stringify.py
	$(CD) src && $(PYTHON3) stringify.py
	$(CC) -c src/enum.c $(CFLAGS) -o src/enum.o

pktest: pktest.o $(FILES)
	$(CC) $(FILES) $(CFLAGS) pktest.o

optest: $(FILES) $@.o
	$(CC) $(FILES) enum.h $(CFLAGS) $@.o

clean:
	$(RM) *.o src/*.o src/enum.c
