#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $(dirname $0)/../functions.sh

for i in {1..10}; do
    network_connect 0 1
    server_start 0
    nic_add 0 1 42 sg-1
    nic_add 0 2 42 sg-1
    nic_add 0 3 42 sg-1
    nic_add 0 4 42 sg-1
    sg_rule_add_all_open 0 sg-1
    server_stop 0
    clean_all
done

