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

script: ../elk/elk.o test/script.o
script: FILES+=../elk/elk.o test/script.o
script: CFLAGS+=-I../elk/ -Isrc/
pktest: test/pktest.o
pktest: FILES+=test/pktest.o
optest: test/optest.o
optest: FILES+=test/optest.o
live: test/live.o
live: FILES+=test/live.o
live: CFLAGS+=-lX11

live script pktest optest: $(FILES) $(info $(FILES))
	$(CC) $(FILES) $(CFLAGS) -o $@

clean:
	$(RM) *.o src/*.o src/data.c *.out optest pktest live script test/*.o
