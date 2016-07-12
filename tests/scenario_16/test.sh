#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
add_nic_port_open 0 1 42 17 5554
add_nic_port_open 0 2 42 17 5554
qemu_start 1
qemu_start 2
ssh_udp 2 1 5554
ssh_udp 1 2 5554
ssh_no_tcp 1 2 4445
ssh_no_tcp 1 2 5554
ssh_no_udp 2 1 1002
ssh_no_udp 1 2 1234
ssh_no_tcp 2 1 5895
ssh_no_udp 1 2 5553
ssh_no_udp 2 1 5555
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result
