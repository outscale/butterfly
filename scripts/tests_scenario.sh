#!/bin/sh

# Butterfly root
BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_TYPE_TEST=$2
VERBOSE=$3
BUTTERFLY_ROOT=$(cd "$(dirname $0)/.." && pwd)

function run_scenario {
    path_to_test=$1
    name_test=$2
    BUTTERFLY_BUILD_ROOT=$3
    VERBOSE=$4
    echo verbose mode $VERBOSE
    for s in $path_to_test/scenario_*; do
        echo "=== running $name_test $(basename $s) ==="
        if [ ".$VERBOSE" == ".1" ]; then
            echo verbose mode
            bash -x $s/test.sh $BUTTERFLY_BUILD_ROOT || exit
        else
            $s/test.sh $BUTTERFLY_BUILD_ROOT || exit 1
        fi
    done
}

# Test Butterfly build root
if [ ! -f $BUTTERFLY_BUILD_ROOT/CMakeCache.txt ]; then
    echo "Wrong build directory ?"
    exit 1
fi

# API tests
if [ "$BUTTERFLY_TYPE_TEST" = "api" ] || [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario "$BUTTERFLY_ROOT/tests/api" "api" $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
# Test Protocol ICMP
if [ "$BUTTERFLY_TYPE_TEST" = "icmp" ] || [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario "$BUTTERFLY_ROOT/tests/network/bypass/icmp" "icmp" $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
# Test Protocol TCP
if [ "$BUTTERFLY_TYPE_TEST" = "tcp" ] || [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario "$BUTTERFLY_ROOT/tests/network/bypass/tcp" "tcp" $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
# Test Protocol UDP
if [ "$BUTTERFLY_TYPE_TEST" = "udp" ] || [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario "$BUTTERFLY_ROOT/tests/network/bypass/udp" "udp" $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
# Test Protocol SCTP
if [ "$BUTTERFLY_TYPE_TEST" = "sctp" ] || [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario "$BUTTERFLY_ROOT/tests/network/bypass/sctp" "sctp" $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
# Test Protocol UDP & TCP
if [ "$BUTTERFLY_TYPE_TEST" = "udp_tcp" ] || [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario "$BUTTERFLY_ROOT/tests/network/bypass/udp_tcp" "udp_tcp" $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
# Test Firewall
if [ "$BUTTERFLY_TYPE_TEST" = "firewall" ] || [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario "$BUTTERFLY_ROOT/tests/network/firewall" "firewall" $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
# Test Other
if [ "$BUTTERFLY_TYPE_TEST" = "other" ] || [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario "$BUTTERFLY_ROOT/tests/other" "other" $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
