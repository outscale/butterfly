#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add_port_open udp sg-1 0 1 42 8000
nic_add_void 0 2 42
qemu_start 1
qemu_start 2

sg_rule_add_port_open udp sg-2 0 9000
ssh_connection_test udp 1 2 8000
ssh_no_connection_test udp 1 2 9000

nic_set_sg 0 1 sg-1 sg-2
ssh_connection_test udp 1 2 8000
ssh_connection_test udp 1 2 9000

remove_sg_from_nic 0 1
nic_set_sg 0 1 sg-2
ssh_no_connection_test udp 1 2 8000
ssh_connection_test udp 1 2 9000

remove_sg_from_nic 0 1
ssh_no_connection_test udp 1 2 8000
ssh_no_connection_test udp 1 2 9000

qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result
