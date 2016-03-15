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

# API tests
$BUTTERFLY_ROOT/scripts/tests_api.sh $BUTTERFLY_ROOT
if [ $? != 0 ]; then
    tput setaf 1
    echo "API test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "API test OK"
    tput setaf 7
fi

sleep 5

# API style test
$BUTTERFLY_ROOT/scripts/tests_api_style.sh $BUTTERFLY_ROOT
if [ $? != 0 ]; then
    tput setaf 1
    echo "API style test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "API style test OK"
    tput setaf 7
fi

sleep 5

# Scenario test
$BUTTERFLY_ROOT/scripts/tests_scenario.sh $BUTTERFLY_ROOT
if [ $? != 0 ]; then
    tput setaf 1
    echo "API scenario test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "API scenario test OK"
    tput setaf 7
fi


tput setaf 2
echo "All test succeded"
tput setaf 7
exit 0

