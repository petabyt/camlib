-include config.mak

CD?=cd
PYTHON3?=python3
FILES=src/libusb.o src/operations.o src/packet.o src/deviceinfo.o src/data.o src/enum.o

CFLAGS=-lusb -Isrc/

%.o: %.c src/*.h
	$(CC) -c $(CFLAGS) $< -o $@

src/data.o: src/ptp.h src/canon.h src/stringify.py
	$(CD) src && $(PYTHON3) stringify.py
	$(CC) -c src/data.c $(CFLAGS) -o src/data.o

pktest: pktest.o $(FILES)
	$(CC) $(FILES) $(CFLAGS) pktest.o -o pktest

optest: $(FILES) $@.o
	$(CC) $(FILES) enum.h $(CFLAGS) optest

clean:
	$(RM) *.o src/*.o src/enum.c *.out optest pktest
