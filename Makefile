-include config.mak

CD?=cd
PYTHON3?=python3

# All platforms need these object files
FILES=$(addprefix src/,operations.o packet.o enum.o data.o enum_dump.o util.o canon.o liveview.o)

# Basic support for MinGW
# backend.o is unusable for Windows WIA, so it can't be linked in.
ifdef WIN
FILES+=src/winapi.o
CC=x86_64-w64-mingw32-gcc
LDFLAGS=-lhid -lole32 -luser32 -lgdi32 -luuid
else
LDFLAGS=-lusb
FILES+=src/libusb.o src/backend.o
endif

CFLAGS=-Isrc/ -I../mjs/ -DVERBOSE -Wall -g

%.o: %.c src/*.h
	$(CC) -c $(CFLAGS) $< -o $@

# Defining NOPYTHON will prevent Python from generating a new file
ifndef NOPYTHON
src/enum_dump.o: src/ptp.h src/stringify.py
	$(CD) src && $(PYTHON3) stringify.py
	$(CC) -c src/enum_dump.c $(CFLAGS) -o src/enum_dump.o
endif

# Defining BIND will compile JSON bindings (bind.c)
ifdef BIND
FILES+=src/bind.o
endif

# PTP decoder
DEC_FILES=src/dec/main.o src/enum.o src/enum_dump.o
dec: $(DEC_FILES)
	$(CC) $(DEC_FILES) $(LDFLAGS) $(CFLAGS) -o $@

# Some basic tests - files need to be added as a dependency
# and also added to the FILES object list
TEST_TARGETS=live script pktest optest test2 evtest wintest.exe bindtest
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
bindtest: test/bindtest.o
bindtest: FILES+=test/bindtest.o
live: test/live.o
live: FILES+=test/live.o
live: CFLAGS+=-lX11
wintest.exe: FILES+=test/wintest.o
wintest.exe: test/wintest.o

$(TEST_TARGETS): $(FILES)
	$(CC) $(FILES) $(LDFLAGS) $(CFLAGS) -o $@

clean:
	$(RM) *.o src/*.o src/dec/*.o *.out $(TEST_TARGETS) test/*.o *.exe *.txt dec
