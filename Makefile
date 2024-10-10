-include config.mak

CFLAGS := -Isrc/ -g -fpic -Wall -Wshadow -Wcast-qual -Wpedantic -Werror=incompatible-pointer-types -Werror=deprecated-declarations
CFLAGS += -D CAMLIB_NO_COMPAT -D VERBOSE

# Camlib needs to be compiled with these files, with some exceptions:
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

all: $(SO)

TARGET ?= l
ifeq ($(TARGET),m) 
SO = libcamlib.dylib
CFLAGS += $(UNIX_CFLAGS)
libcamlib.dylib: $(UNIX_LIB_FILES)
	$(CC) -shared $(UNIX_LIB_FILES) -L/usr/local/lib $(UNIX_CFLAGS) $(UNIX_LDFLAGS) -o libcamlib.dylib
endif
ifeq ($(TARGET),l)
SO = libcamlib.a
CFLAGS += $(UNIX_CFLAGS)
libcamlib.a: $(UNIX_LIB_FILES)
	ar rcs libcamlib.a  $(UNIX_LIB_FILES)
LDFLAGS := $(UNIX_LDFLAGS)
endif
ifeq ($(TARGET),w)
SO = libcamlib.dll
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

-include src/*.d src/lua/*.d src/lua/lua-cjson/*.d

camlib: src/cli.o src/dec/main.o $(SO)
	$(CC) src/cli.o src/dec/main.o $(CFLAGS) $(SO) $(LDFLAGS) -o $@

# Run this thing frequently
stringify:
	python3 stringify.py

clean:
	rm -rf *.o src/*.o src/dec/*.o *.out test-ci test/*.o examples/*.o examples/*.d *.exe camlib *.dll *.so DUMP \
	src/lua/*.o src/lua/lua-cjson/*.o src/*.d examples/*.d src/lua/*.d src/lua/lua-cjson/*.d
	cd examples && make clean

install: libcamlib.so
	cp libcamlib.so /usr/lib/
	-mkdir /usr/include/camlib
	cp src/*.h /usr/include/camlib/

test-ci: test/test.o test/data.o $(UNIX_LIB_FILES)
	$(CC) test/test.o test/data.o $(UNIX_LIB_FILES) -lusb-vcam -lexif $(CFLAGS) -o test-ci

test: test-ci
	./test-ci

.PHONY: all clean install stringify test
