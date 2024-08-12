# Unix only makefile
-include config.mak

CFLAGS := -Isrc/ -g -fpic -Wall -Wshadow -Wcast-qual -Wpedantic -Werror=incompatible-pointer-types -Werror=deprecated-declarations
CFLAGS += -D CAMLIB_NO_COMPAT -D VERBOSE

# All platforms need these object files
CAMLIB_CORE := operations.o packet.o enums.o data.o enum_dump.o lib.o canon.o liveview.o bind.o ip.o ml.o log.o conv.o generic.o canon_adv.o
FILES := $(addprefix src/,$(CAMLIB_CORE))

EXTRAS := src/canon_adv.o

# Unix-specific
CFLAGS += $(shell pkg-config --cflags libusb-1.0)
LDFLAGS ?= $(shell pkg-config --libs libusb-1.0)
FILES += src/libusb.o src/transport.o

libcamlib.so: $(FILES) $(EXTRAS)
	$(CC) -shared $(FILES) -o libcamlib.so

# MacOS dylib
ifeq ($(TARGET),m)
CFLAGS += -I/usr/local/include/libusb-1.0
libcamlib.dylib: $(FILES)
	$(CC) -shared $(FILES) -L/usr/local/lib `pkg-config --cflags --libs libusb` -o libcamlib.so
endif

%.o: %.c
	$(CC) -MMD -c $(CFLAGS) $< -o $@

-include src/*.d lua/*.d lua/lua-cjson/*.d

# PTP decoder
DEC_FILES := src/dec/main.o src/enums.o src/enum_dump.o src/packet.o src/conv.o src/log.o
dec: $(DEC_FILES)
	$(CC) $(DEC_FILES) $(CFLAGS) -o $@

# Run this thing frequently
stringify:
	python3 stringify.py

clean:
	rm -rf *.o src/*.o src/dec/*.o *.out test-ci test/*.o examples/*.o examples/*.d *.exe dec *.dll *.so DUMP \
	lua/*.o lua/lua-cjson/*.o src/*.d examples/*.d lua/*.d lua/lua-cjson/*.d
	cd examples && make clean

install: libcamlib.so
	cp libcamlib.so /usr/lib/
	-mkdir /usr/include/camlib
	cp src/*.h /usr/include/camlib/

test-ci: test/test.o $(FILES) ../vcam/libusb.so
	$(CC) test/test.o $(FILES) -L../vcam/ -Wl,-rpath=../vcam/ -lusb -lexif $(CFLAGS) -o test-ci

test: test-ci
	./test-ci

.PHONY: all clean install stringify test
