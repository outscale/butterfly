#!/bin/bash

BT_BUILD=$1

cd $BT_BUILD
g++ -std=c++11 -lcrypto -DUNIT_TESTS -I../ ../api/common/crypto.cc -o crypto
./crypto
RET=$?
rm crypto
if [ $RET != 0 ]; then
    exit 1
fi
