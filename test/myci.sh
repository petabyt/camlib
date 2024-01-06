#!/bin/sh

cp test/myci-config.mak config.mak
cd ../vcam/
make libusb.so
cd ../camlib
make test-ci && ./test-ci
echo "test return value: $?"

rm config.mak
