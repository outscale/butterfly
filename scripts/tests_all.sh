#!/bin/bash

# Butterfly root
BUTTERFLY_BUILD_ROOT=$1
VERBOSE=$2
BUTTERFLY_ROOT=$(cd "$(dirname $0)/.." && pwd)

# Test Butterfly build root
if [ ! -f $BUTTERFLY_BUILD_ROOT/CMakeCache.txt ]; then
    echo "Wrong build directory ?"
    exit 1
fi

#Clean coverage data
sudo find $BUTTERFLY_BUILD_ROOT/api/ -name '*.gcda' -exec rm -fr {} \;

make all

echo "############################################################"
echo "Butterfly tests start"
echo $($BUTTERFLY_BUILD_ROOT/api/server/butterflyd --version)
echo "############################################################"

# API style test
$BUTTERFLY_ROOT/scripts/tests_api_style.sh
if [ $? != 0 ]; then
    echo "API style test failed"
    exit 1
else
    echo "API style test OK"
fi

sleep 1

# API tests
$BUTTERFLY_ROOT/scripts/tests_api.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    echo "API test failed"
    exit 1
else
    echo "API test OK"
fi

sleep 5

# Scenario test
$BUTTERFLY_ROOT/scripts/tests_scenario.sh $BUTTERFLY_BUILD_ROOT all $VERBOSE
if [ $? != 0 ]; then
    echo "API scenario test failed"
    exit 1
else
    echo "API scenario test OK"
fi

# API coverage test
check_files=$(find $BUTTERFLY_BUILD_ROOT/api/ -name '*.gcda' | wc -l)
if [ $check_files -gt 0 ]; then
    $BUTTERFLY_ROOT/scripts/tests_coverage.sh $BUTTERFLY_BUILD_ROOT
    if [ $? != 0 ]; then
        echo "API coverage test failed"
       exit 1
    else
        echo "API coverage test OK"
    fi
fi

echo "All test succeded"
exit 0

