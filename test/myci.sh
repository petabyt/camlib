#!/bin/sh

cp test/myci-config.mak config.mak
cd ../vusb/
make libusb.so
mv libusb.so libvusb.so
cd ../camlib
make test-ci
valgrind ./test-ci

rm config.mak
