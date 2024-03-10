#!/bin/sh
set -e 
set -o pipefail

cp test/ci-config.mak config.mak
cd ..
# In the future, this will be a fixed branch or stable release
git clone https://github.com/petabyt/vcam
cd vcam
make libusb.so
cd ../camlib
make test-ci
./test-ci
