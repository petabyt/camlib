-include config.mak

CC=gcc
CD?=cd
PYTHON3?=python3
FILES=$(addprefix src/,libusb.o operations.o packet.o enum.o data.o enum_dump.o util.o canon.o backend.o generic.o)

CFLAGS=-Isrc/ -I../mjs/
LDFLAGS=-lusb

%.o: %.c src/*.h
	$(CC) -c $(CFLAGS) $< -o $@

src/enum_dump.o: src/ptp.h src/stringify.py
	$(CD) src && $(PYTHON3) stringify.py
	$(CC) -c src/enum_dump.c $(CFLAGS) -o src/enum_dump.o

# Some deps/tweaks for tests
TEST_TARGETS=live script pktest optest test2 evtest
script: ../mjs/mjs.o test/script.o
script: FILES+=../mjs/mjs.o test/script.o
pktest: test/pktest.o
pktest: FILES+=test/pktest.o
optest: test/optest.o
optest: FILES+=test/optest.o
test2: test/test2.o
test2: FILES+=test/test2.o
evtest: test/evtest.o
evtest: FILES+=test/evtest.o
live: test/live.o
live: FILES+=test/live.o
live: CFLAGS+=-lX11

$(TEST_TARGETS): $(FILES)
	$(CC) $(FILES) $(LDFLAGS) $(CFLAGS) -o $@

script_test: script
	./script test/connect.js

clean:
	$(RM) *.o src/*.o *.out $(TEST_TARGETS) test/*.o
