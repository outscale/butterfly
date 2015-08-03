#!/bin/sh

# Butterfly root
BUTTERFLY_ROOT=$1
BUTTERFLY_BUILD_ROOT=.

# Test Butterfly build root
if [ ! -f $BUTTERFLY_BUILD_ROOT/CMakeCache.txt ]; then
    echo "Please run script from the build directory"
    exit 1
fi

# Test Butterfly root
if [ ! -d $BUTTERFLY_ROOT/api ]; then
    echo "Please set butterfly's source root as parameter"
    exit 1
fi

# Launch the tests suites of all the subprojects

GREEN="\033[32m"
RED="\033[31m"
NORMAL="\033[0m"

# API tests
$BUTTERFLY_ROOT/scripts/tests_api.sh $BUTTERFLY_ROOT
if [ $? != 0 ]; then
    echo "${RED}API test failed${NORMAL}"
    exit 1
else
    echo "${GREEN}API test OK${NORMAL}"
fi

# API style test
$BUTTERFLY_ROOT/scripts/tests_api_style.sh $BUTTERFLY_ROOT
if [ $? != 0 ]; then
    echo "${RED}API style test failed${NORMAL}"
    exit 1
else
    echo "${GREEN}API style test OK${NORMAL}"
fi

echo "${GREEN}All test succeded${NORMAL}"
exit 0

