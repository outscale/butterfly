#!/bin/bash

# Butterfly root
BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_TYPE_TEST=$2
VERBOSE=$3
BUTTERFLY_ROOT=$(cd "$(dirname $0)/.." && pwd)

function run_scenario {
    BUTTERFLY_BUILD_ROOT=$1
    VERBOSE=$2
    for s in $BUTTERFLY_ROOT/tests/*/; do
        ret=$(find $s -name '*api*' | wc -l)
        if [ $ret -eq 0 ]; then
            echo "=== running $(basename $s) ==="
            if [ ".$VERBOSE" == ".1" ]; then
                echo verbose mode
                bash -x $s/test.sh $BUTTERFLY_BUILD_ROOT || exit
            else
                $s/test.sh $BUTTERFLY_BUILD_ROOT || exit 1
            fi
        fi
    done
}

function run_target_scenario {
    BUTTERFLY_BUILD_ROOT=$1
    target=$2
    VERBOSE=$3
    for s in $BUTTERFLY_ROOT/tests/*/; do
        ret=$(find $s -name '*'$target'*' -type d | wc -l)
        if [ $ret -eq 1 ]; then
            echo "=== running $(basename $s) ==="
            if [ ".$VERBOSE" == ".1" ]; then
                echo verbose mode
                bash -x $s/test.sh $BUTTERFLY_BUILD_ROOT || exit
            else
                $s/test.sh $BUTTERFLY_BUILD_ROOT || exit 1
            fi
        fi
    done
}

function run_other_scenario {
    BUTTERFLY_BUILD_ROOT=$1
    VERBOSE=$2
    for s in $BUTTERFLY_ROOT/tests/*/; do
        ret=$(find $s -name '*ping*' -o -name '*tcp*' -o -name '*udp*' -o -name '*sctp*' -o -name '*firewall*' -o -name '*api*' -type d | wc -l)
        if [ $ret -eq 0 ]; then
            echo "=== running $(basename $s) ==="
            if [ ".$VERBOSE" == ".1" ]; then
                echo verbose mode
                bash -x $s/test.sh $BUTTERFLY_BUILD_ROOT || exit
            else
                $s/test.sh $BUTTERFLY_BUILD_ROOT || exit 1
            fi
        fi
    done
}

# Test Butterfly build root
if [ ! -f $BUTTERFLY_BUILD_ROOT/CMakeCache.txt ]; then
    echo "Wrong build directory ?"
    exit 1
fi

# Test Protocol ICMP
if [ "$BUTTERFLY_TYPE_TEST" = "icmp" ]; then
    run_target_scenario $BUTTERFLY_BUILD_ROOT "ping" $VERBOSE
fi
# Test Protocol TCP
if [ "$BUTTERFLY_TYPE_TEST" = "tcp" ]; then
    run_target_scenario $BUTTERFLY_BUILD_ROOT "tcp" $VERBOSE
fi
# Test Protocol UDP
if [ "$BUTTERFLY_TYPE_TEST" = "udp" ]; then
    run_target_scenario $BUTTERFLY_BUILD_ROOT "udp" $VERBOSE
fi
# Test Protocol SCTP
if [ "$BUTTERFLY_TYPE_TEST" = "sctp" ]; then
    run_target_scenario $BUTTERFLY_BUILD_ROOT "sctp" $VERBOSE
fi
# Test Protocol UDP & TCP
if [ "$BUTTERFLY_TYPE_TEST" = "udp_tcp" ]; then
    run_target_scenario $BUTTERFLY_BUILD_ROOT "udp_tcp" $VERBOSE
fi
# Test Firewall
if [ "$BUTTERFLY_TYPE_TEST" = "firewall" ]; then
    run_target_scenario $BUTTERFLY_BUILD_ROOT "firewall" $VERBOSE
fi
# Test other
if [ "$BUTTERFLY_TYPE_TEST" = "other" ]; then
    run_other_scenario $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
#Test All
if [ "$BUTTERFLY_TYPE_TEST" = "all" ]; then
    run_scenario $BUTTERFLY_BUILD_ROOT $VERBOSE
fi
