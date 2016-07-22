#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
add_nic sg-1 0 1 42
add_nic sg-1 0 2 42
add_nic sg-1 1 3 42
add_nic sg-1 1 4 42
qemu_start 1
qemu_start 2
qemu_start 3
qemu_start 4
ssh_connection_test udp 2 1 1234
ssh_connection_test udp 3 1 5554
ssh_connection_test udp 4 1 4123
ssh_connection_test udp 1 2 1234
ssh_connection_test udp 3 2 5554
ssh_connection_test udp 4 2 4123
ssh_connection_test udp 1 3 1234
ssh_connection_test udp 2 3 5554
ssh_connection_test udp 4 3 4123
ssh_connection_test udp 1 4 5004
ssh_connection_test udp 2 4 5964
ssh_connection_test udp 3 4 1004
qemu_stop 1
qemu_stop 2
qemu_stop 3
qemu_stop 4
server_stop 0
server_stop 1
network_disconnect 0 1
return_result

