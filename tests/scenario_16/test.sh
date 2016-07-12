#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
add_nic_port_open tcp sg-1 0 1 42 4454
add_nic_port_open tcp sg-1 0 2 42 4454
qemu_start 1
qemu_start 2
ssh_connection_test tcp 2 1 4454
ssh_connection_test tcp 1 2 4454
ssh_no_connection_test tcp 1 2 4452
ssh_no_connection_test tcp 1 2 4453
ssh_no_connection_test udp 2 1 4454
ssh_no_connection_test tcp 1 2 1234
ssh_no_connection_test udp 2 1 5895
ssh_no_connection_test tcp 1 2 5053
ssh_no_connection_test udp 2 1 5505
qemu_stop 1
qemu_stop 2
sleep 0.2
server_stop 0
network_disconnect 0 1
return_result

