#!/bin/sh

cp test/myci-config.mak config.mak
cd ../vusb/
make libusb.so
mv libusb.so libvusb.so
cd ../camlib
make test-ci && ./test-ci
echo "test return value: $?"

rm config.mak
