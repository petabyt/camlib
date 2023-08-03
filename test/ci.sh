#!/bin/sh

cp test/ci-config.mak config.mak
git clone https://github.com/petabyt/libusb-fake-ptp
cd libusb-fake-ptp
make libusb.so
mv libusb.so libvusb.so
cd ..
make test-ci
./test-ci
