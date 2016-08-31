#!/bin/sh

# Butterfly root
BUTTERFLY_ROOT=$1

# Test Butterfly root
if [ ! -d $BUTTERFLY_ROOT/api ]; then
    echo "Please set butterfly's source root as parameter"
    exit 1
fi

sources="$BUTTERFLY_ROOT/api/client/client.cc \
$BUTTERFLY_ROOT/api/client/request.cc \
$BUTTERFLY_ROOT/api/client/all_infos.cc \
$BUTTERFLY_ROOT/api/server/app.cc \
$BUTTERFLY_ROOT/api/server/app.h \
$BUTTERFLY_ROOT/api/server/server.cc \
$BUTTERFLY_ROOT/api/server/server.h \
$BUTTERFLY_ROOT/api/server/model.cc \
$BUTTERFLY_ROOT/api/server/model.h \
$BUTTERFLY_ROOT/api/server/api.cc \
$BUTTERFLY_ROOT/api/server/api.h \
$BUTTERFLY_ROOT/api/server/api_0.cc \
$BUTTERFLY_ROOT/api/server/pg.cc \
$BUTTERFLY_ROOT/api/server/pg.h \
$BUTTERFLY_ROOT/api/server/graph.cc \
$BUTTERFLY_ROOT/api/server/graph.h"

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

exit 0
