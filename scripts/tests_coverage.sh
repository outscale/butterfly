#!/bin/sh

# Butterfly build root
BUTTERFLY_BUILD_ROOT=$1
client_obj_dir=$BUTTERFLY_BUILD_ROOT/api/client/CMakeFiles/api_client.dir/
server_obj_dir=$BUTTERFLY_BUILD_ROOT/api/server/CMakeFiles/api_server.dir/

# Test Butterfly build root
if [ ! -d $client_obj_dir -o ! -d $server_obj_dir ]; then
    echo "either '$client_obj_dir' or '$server_obj_dir' is not a directory, have you set BUTTERFLY_BUILD_ROOT ?"
    exit 1
fi

# Test coverage data files
ret=$(find $BUTTERFLY_BUILD_ROOT/api/ -name '*.gcda' | wc -l)
if [ $ret -eq 0 ]; then
    echo " Coverage files (*.gcda) not found in $client_obj_dir"
    echo " Make sure you did that in your build directory:"
    echo " - cmake .. -DCOVERAGE=ON"
    echo " - make"
    echo " run some tests before running coverage test."
    exit 1
fi

rm -fr $BUTTERFLY_BUILD_ROOT/client_coverage $BUTTERFLY_BUILD_ROOT/server_coverage

function check_error {
if [ $1 -ne 0 ]; then
    exit 1
fi
}

function clean_coverage_data {
    find $BUTTERFLY_BUILD_ROOT/api/ -name '*.gcda' -exec rm -fr {} \;
    check_error $?
    rm client.info server.info
}

lcov -h >/dev/null || (echo "ERROR: lcov command not found" ; exit 1)
lcov -c -d $client_obj_dir -o client.info
lcov -c -d $server_obj_dir -o server.info
genhtml $BUTTERFLY_BUILD_ROOT/server.info -o server_coverage > /dev/null
check_error $?
genhtml $BUTTERFLY_BUILD_ROOT/client.info -o client_coverage > /dev/null
check_error $?
clean_coverage_data

echo COVERAGE TEST OK 
exit 0
