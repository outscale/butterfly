#!/bin/sh

# Butterfly root
BUTTERFLY_ROOT=$1
VERBOSE=$2
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

#remove out.txt
sudo rm out.txt

source $BUTTERFLY_ROOT/api/protocol/version

run=$BUTTERFLY_ROOT/api/tests/run_scenario.sh
client=$BUTTERFLY_BUILD_ROOT/api/client/butterfly
server=$BUTTERFLY_BUILD_ROOT/api/server/butterflyd
expected=$BUTTERFLY_BUILD_ROOT/api_test_out
err=false
for request in $BUTTERFLY_ROOT/api/tests/*/*_in; do
    tput setaf 2
    echo -n "$(basename $request) scenario: "
    tput setaf 7
    expected_raw=$(echo $request | sed -e 's/_in$/_out/')
    cat $expected_raw |  sed -e "s/PROTO_REVISION/$PROTO_REVISION/" > $expected
    if [ ".$VERBOSE" == ".1" ]; then
        sudo bash -x $run $client $server $request $expected
    else
        sudo $run $client $server $request $expected
    fi
    if [ $? -ne 0 ]; then
	tput setaf 1
        echo FAILED !
        err=true
	tput setaf 7
	exit 1
    else
	tput setaf 2
        echo OK
	tput setaf 7
    fi
done

sudo chown $USER out.txt

if [ $err = true ]; then
    exit 1
fi
exit 0
