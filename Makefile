-include config.mak

CD?=cd
PYTHON3?=python3

# All platforms need these object files
FILES=$(addprefix src/,operations.o packet.o enums.o data.o enum_dump.o util.o canon.o liveview.o bind.o)

CFLAGS = -Isrc/ -I../mjs/ -DVERBOSE -Wall -g -fpic

# Basic support for MinGW and libwpd
ifdef WIN
  FILES+=src/libwpd.o
  CC=x86_64-w64-mingw32-gcc
  LDFLAGS=-lhid -lole32 -luser32 -lgdi32 -luuid libwpd.dll
else
  CFLAGS += $(shell pkg-config --cflags libusb-1.0)
  LDFLAGS += $(shell pkg-config --libs libusb-1.0)
  FILES+=src/libusb.o src/backend.o
endif


# TODO: implement as CAMLIB_VERSION
COMMIT=$(shell git rev-parse HEAD)

libcl.so: $(FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared $(FILES) -o libcl.so

%.o: %.c src/*.h
	$(CC) -c $(CFLAGS) $< -o $@

# Defining NOPYTHON will prevent Python from generating a new file
ifndef NOPYTHON
src/enum_dump.o: src/ptp.h src/stringify.py
	$(CD) src && $(PYTHON3) stringify.py
	$(CC) -c src/enum_dump.c $(CFLAGS) -o src/enum_dump.o
endif

# PTP decoder
DEC_FILES=src/dec/main.o src/enums.o src/enum_dump.o
dec: $(DEC_FILES)
	$(CC) $(DEC_FILES) $(CFLAGS) -o $@

# Some basic tests - files need to be added as a dependency
# and also added to the FILES object list
TEST_TARGETS=live script pktest optest test2 evtest wintest.exe bindtest fh
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
	$(RM) *.o src/*.o src/dec/*.o *.out $(TEST_TARGETS) test/*.o *.exe *.txt dec *.dll *.so

.PHONY: all clean
