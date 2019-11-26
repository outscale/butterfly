#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
CUR_TEST="$(dirname $0)"
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

LOOP_NB=0

for i in {1..10}; do
    echo "+===================================================+"
    echo "$CUR_TEST ( $LOOP_NB )"
    echo "+===================================================+"
    network_connect 0 1
    server_start 0
    nic_add 0 1 42 sg-1
    nic_add 0 2 42 sg-1
    nic_add 0 3 42 sg-1
    nic_add 0 4 42 sg-1
    sg_rule_add_all_open 0 sg-1
    server_stop 0
    clean_all
    LOOP_NB=$(($LOOP_NB + 1))
done

