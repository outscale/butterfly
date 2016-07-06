#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
add_nic_no_sg 0 1 42
add_nic_full_open 0 2 42
qemu_start 1
qemu_start 2
ssh_udp 2 1 1243
ssh_tcp 2 1 5543
ssh_no_udp 1 2 1234
ssh_no_tcp 1 2 1254
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result

