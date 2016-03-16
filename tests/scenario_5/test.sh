#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

for i in {1..10}; do
    network_connect 0 1
    server_start 0
    client_add_nic 0 1 42
    client_add_nic 0 2 42
    client_add_nic 0 3 42
    client_add_nic 0 4 42
    server_stop 0
    clean_all
done

