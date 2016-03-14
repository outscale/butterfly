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

for s in $BUTTERFLY_ROOT/tests/scenario_*; do
    echo "=== running $(basename $s) ==="
    $s/test.sh $BUTTERFLY_ROOT $BUTTERFLY_BUILD_ROOT || exit 1
done
