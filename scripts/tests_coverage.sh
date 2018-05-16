#!/bin/bash

# Butterfly build root
BUTTERFLY_BUILD_ROOT=$1
client_obj_dir=$BUTTERFLY_BUILD_ROOT/api/client/CMakeFiles/api_client.dir/
server_obj_dir=$BUTTERFLY_BUILD_ROOT/api/server/CMakeFiles/api_server.dir/
client_src_dir=$BUTTERFLY_ROOT/api/client/
server_src_dir=$BUTTERFLY_ROOT/api/server/

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

function clean_coverage_data {
    find $BUTTERFLY_BUILD_ROOT/api/ -name '*.gcda' -exec rm -fr {} \; || exit 1
    rm client.info server.info >/dev/null
}

# Coverage test to get html files at output
lcov -h >/dev/null || (echo "ERROR: lcov command not found" ; exit 1)
genhtml -h >/dev/null || (echo "ERROR: lcov command not found" ; exit 1)
lcov -c -d $client_obj_dir -o client.info
lcov -c -d $server_obj_dir -o server.info
genhtml $BUTTERFLY_BUILD_ROOT/server.info -o server_coverage > /dev/null || exit 1
genhtml $BUTTERFLY_BUILD_ROOT/client.info -o client_coverage > /dev/null || exit 1

# Test coverage for cobertura report on jenkins
gcovr -h >/dev/null || (echo "ERROR: gcovr command not found" ; exit 1)
cd $server_src_dir
gcovr -r . --branches --exclude-directories=simpleini --object-directory=$server_obj_dir --xml-pretty -o ser_coverage.xml
cd $client_src_dir
gcovr -r . --branches --object-directory=$client_obj_dir --xml-pretty -o cli_coverage.xml
cd $BUTTERFLY_BUILD_ROOT

clean_coverage_data

echo COVERAGE TEST OK 
exit 0
