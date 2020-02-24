#!/bin/bash

BT_BUILD=$1

cd $BT_BUILD
g++ ../api/common/crypto.cc -std=c++11 -lcrypto -DUNIT_TESTS -I ../ -o crypto
./crypto
RET=$?
rm crypto
if [ $RET != 0 ]; then
    exit 1
fi
