#!/bin/bash

# Butterfly root
BUTTERFLY_ROOT=$(cd "$(dirname $0)/.." && pwd)

sources="$BUTTERFLY_ROOT/api/client/client.cc \
$BUTTERFLY_ROOT/api/client/client.h \
$BUTTERFLY_ROOT/api/client/nic.cc \
$BUTTERFLY_ROOT/api/client/request.cc \
$BUTTERFLY_ROOT/api/client/sg.cc \
$BUTTERFLY_ROOT/api/client/shutdown.cc \
$BUTTERFLY_ROOT/api/client/status.cc \
$BUTTERFLY_ROOT/api/client/dump.cc \
$BUTTERFLY_ROOT/api/server/app.cc \
$BUTTERFLY_ROOT/api/server/app.h \
$BUTTERFLY_ROOT/api/server/server.cc \
$BUTTERFLY_ROOT/api/server/server.h \
$BUTTERFLY_ROOT/api/server/model.cc \
$BUTTERFLY_ROOT/api/server/model.h \
$BUTTERFLY_ROOT/api/server/api.cc \
$BUTTERFLY_ROOT/api/server/api.h \
$BUTTERFLY_ROOT/api/server/api_0.cc \
$BUTTERFLY_ROOT/api/server/graph.cc \
$BUTTERFLY_ROOT/api/server/graph.h \
$BUTTERFLY_ROOT/api/common/crypto.cc \
$BUTTERFLY_ROOT/api/common/crypto.h"

$BUTTERFLY_ROOT/scripts/cpplint.py --filter=-build/c++11 --root=$BUTTERFLY_ROOT $sources
if [ $? != 0 ]; then
    echo "${RED}API style test failed${NORMAL}"
    exit 1
fi

cppcheck &> /dev/null
if [ $? != 0 ]; then
    echo "cppcheck is not installed, some tests will be skipped"
else
    cppcheck --check-config --error-exitcode=1 --enable=all -I $BUTTERFLY_ROOT $sources &> /tmp/cppcheck.log
    if [ $? != 0 ]; then
        cat /tmp/cppcheck.log
        echo "${RED}API style test failed${NORMAL}"
        rm /tmp/cppcheck.log
        exit 1
    fi
fi
rm /tmp/cppcheck.log

rm /tmp/has_tabs &> /dev/null || true
for f in api benchmarks doc scripts tests; do
    find $BUTTERFLY_ROOT/$f -name *.sh | while read a; do
        if [ "-$(cat $a | grep -P '\t')" != "-" ]; then
            echo found tab in $a
            touch /tmp/has_tabs
        fi
    done
done
if test -f /tmp/has_tabs; then
    rm /tmp/has_tabs &> /dev/null || true
    echo "-- tabs found in scripts"
    exit 1
else
    echo "-- no tab found in scripts"
fi

exit 0
