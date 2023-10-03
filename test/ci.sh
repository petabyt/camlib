#!/bin/sh

cp test/ci-config.mak config.mak
cd ..
git clone https://github.com/petabyt/vcam
cd vcam
make libusb.so
mv libusb.so libvusb.so
cd ../camlib
make test-ci
./test-ci
