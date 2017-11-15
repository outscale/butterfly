#!/bin/sh

# Butterfly root
BUTTERFLY_BUILD_ROOT=$1
VERBOSE=$2
BUTTERFLY_ROOT=$(cd "$(dirname $0)/.." && pwd)

# Test Butterfly build root
if [ ! -f $BUTTERFLY_BUILD_ROOT/CMakeCache.txt ]; then
    echo "Wrong build directory ?"
    exit 1
fi

#Test Protocol SCTP
echo verbose mode $VERBOSE
for s in $BUTTERFLY_ROOT/tests/network/bypass/sctp/scenario_*; do
    echo "=== running $(basename $s) ==="
    if [ ".$VERBOSE" == ".1" ]; then
        echo verbose mode
        bash -x $s/test.sh $BUTTERFLY_BUILD_ROOT || exit 1
    else
        $s/test.sh $BUTTERFLY_BUILD_ROOT || exit 1
    fi
done
