# Blah blah, old test stuff

# Some basic tests - files need to be added as a dependency
# and also added to the FILES object list
TEST_TARGETS=live pktest optest evtest wintest.exe bindtest wifi storage directprint
storage: test/storage.o
storage: FILES+=test/storage.o
directprint: test/directprint.o
directprint: FILES+=test/directprint.o
pktest: test/pktest.o
pktest: FILES+=test/pktest.o
wifi: test/wifi.o
wifi: FILES+=test/wifi.o
optest: test/optest.o
optest: FILES+=test/optest.o
evtest: test/evtest.o
evtest: FILES+=test/evtest.o
bindtest: test/bindtest.o
bindtest: FILES+=test/bindtest.o
live: test/live.o
live: FILES+=test/live.o ../tigr/tigr.o
live: CFLAGS+=-lGLU -lGL -lX11 -ljpeg
wintest.exe: FILES+=test/wintest.o
wintest.exe: test/wintest.o

$(TEST_TARGETS): $(FILES)
	$(CC) $(FILES) $(LDFLAGS) $(CFLAGS) -o $@
