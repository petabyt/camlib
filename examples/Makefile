CFLAGS=-I../src/
LDFLAGS=-L../ -Wl,-rpath,../ -lcamlib $(shell pkg-config --libs libusb-1.0)

TEST_TARGETS=live pktest optest evtest wintest.exe bindtest wifi storage directprint info usb
usb: usb.o
usb: FILES+=usb.o
info: info.o
info: FILES+=info.o
storage: storage.o
storage: FILES+=storage.o
directprint: directprint.o
directprint: FILES+=directprint.o
pktest: pktest.o
pktest: FILES+=pktest.o
wifi: wifi.o
wifi: FILES+=wifi.o
optest: optest.o
optest: FILES+=optest.o
evtest: evtest.o
evtest: FILES+=evtest.o
bindtest: bindtest.o
bindtest: FILES+=bindtest.o
live: live.o
live: FILES+=live.o
wintest.exe: FILES+=wintest.o
wintest.exe: wintest.o

TEST_TARGETS+=eos
eos: eos.o 
eos: FILES+=eos.o

TEST_TARGETS+=ml
ml: ml.o
ml: FILES+=ml.o

build-so:
	cd ../ && make libcamlib.so

$(TEST_TARGETS): $(FILES) build-so
	$(CC) $(FILES) $(LDFLAGS) $(CFLAGS) -o $@

all: $(TEST_TARGETS)

clean:
	rm -rf *.o $(TEST_TARGETS) $(FILES) DUMP*
