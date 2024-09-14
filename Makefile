-include config.mak

CFLAGS := -Isrc/ -g -fpic -Wall -Wshadow -Wcast-qual -Wpedantic -Werror=incompatible-pointer-types -Werror=deprecated-declarations
CFLAGS += -D CAMLIB_NO_COMPAT -D VERBOSE

# Camlib needs to be compiled with these, with some exceptions:
# - log.c can be replaced with a custom logging mechanism
# - ip.c can be replaced with no_ip.c
# - libwpd.c or libusb.c can be replaced with no_usb.c
# - bind.c and liveview.c can be omitted together
CAMLIB_CORE := operations.o packet.o enums.o data.o enum_dump.o lib.o canon.o liveview.o bind.o ip.o ml.o conv.o generic.o transport.o log.o
CAMLIB_CORE := $(addprefix src/,$(CAMLIB_CORE))

# Implements CHDK and Magic Lantern functionality
EXTRAS := src/canon_adv.o src/object.o

# Unix-specific
UNIX_CFLAGS = $(shell pkg-config --cflags libusb-1.0)
UNIX_LDFLAGS = $(shell pkg-config --libs libusb-1.0)
UNIX_LIB_FILES := $(CAMLIB_CORE) $(EXTRAS) src/libusb.o
WIN_LIB_FILES := $(CAMLIB_CORE) $(EXTRAS) src/libwpd.o

TARGET ?= l
ifeq ($(TARGET),m) 
all: libcamlib.dylib
CFLAGS += $(UNIX_CFLAGS)
libcamlib.dylib: $(UNIX_LIB_FILES)
	$(CC) -shared $(UNIX_LIB_FILES) -L/usr/local/lib $(UNIX_CFLAGS) $(UNIX_LDFLAGS) -o libcamlib.dylib
endif
ifeq ($(TARGET),l)
all: libcamlib.so
CFLAGS += $(UNIX_CFLAGS)
libcamlib.so: $(UNIX_LIB_FILES)
	$(CC) -shared $(UNIX_LIB_FILES) -o libcamlib.so
endif
ifeq ($(TARGET),w)
all: libcamlib.dll
MINGW := x86_64-w64-mingw32
CC := $(MINGW)-gcc
CPP := $(MINGW)-c++
MINGW_LIBS := -lwpd -luser32 -lkernel32 -lgdi32 -lcomctl32 -luxtheme -lmsimg32 -lcomdlg32 -ld2d1 -ldwrite -lole32 -loleaut32 -loleacc -lstdc++ -lgcc -lpthread -lssp -lurlmon -luuid -lws2_32
libcamlib.dll: $(WIN_LIB_FILES)
	$(CC) -shared $(WIN_LIB_FILES) $(MINGW_LIBS) -o libcamlib.dll
LDFLAGS := $(MINGW_LIBS)
endif

%.o: %.c
	$(CC) -MMD -c $(CFLAGS) $< -o $@

-include src/*.d lua/*.d lua/lua-cjson/*.d

DEC_FILES := src/dec/main.o src/enums.o src/enum_dump.o src/packet.o src/conv.o src/log.o
dec: $(DEC_FILES)
	$(CC) $(DEC_FILES) $(CFLAGS) -o $@

camlib: src/cli.o $(WIN_LIB_FILES)
	$(CC) src/cli.o $(WIN_LIB_FILES) $(CFLAGS) $(LDFLAGS) -o $@

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

test-ci: test/test.o $(UNIX_LIB_FILES)
	$(CC) test/test.o $(UNIX_LIB_FILES) -lusb-vcam -lexif $(CFLAGS) -o test-ci

test: test-ci
	./test-ci

.PHONY: all clean install stringify test
