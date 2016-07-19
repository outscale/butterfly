#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
add_nic_port_open udp sg-1 0 1 42 8000
add_nic_port_open udp sg-1 0 2 42 8000
add_nic_port_open udp sg-1 0 3 42 9000
qemu_start 1
qemu_start 2
qemu_start 3
ssh_connection_test udp 1 2 8000
ssh_connection_test udp 2 1 8000
ssh_connection_test udp 1 3 8000
ssh_connection_test udp 3 2 8000
ssh_no_connection_test tcp 1 2 8000
ssh_no_connection_test tcp 2 1 8000
ssh_no_connection_test udp 3 1 9000
ssh_no_connection_test udp 2 3 9000

add_sg_rule_port_open udp sg-1 0 3 42 8000
ssh_connection_test udp 1 2 8000
ssh_connection_test udp 2 1 8000
ssh_connection_test udp 1 3 8000
ssh_connection_test udp 3 1 8000
ssh_connection_test udp 3 2 8000
ssh_connection_test udp 2 3 8000
ssh_no_connection_test tcp 1 2 8000
ssh_no_connection_test tcp 2 1 8000

delete_sg sg-1 0
ssh_no_connection_test udp 1 2 8000
ssh_no_connection_test udp 2 1 8000
ssh_no_connection_test udp 1 3 8000
ssh_no_connection_test udp 3 1 8000
ssh_no_connection_test udp 3 2 8000
ssh_no_connection_test udp 2 3 8000
ssh_no_connection_test tcp 1 2 8000
ssh_no_connection_test tcp 2 1 8000

add_sg sg-1 0 1
add_sg_rule_port_open udp sg-1 0 1 8000
ssh_connection_test udp 1 2 8000
ssh_connection_test udp 2 1 8000
ssh_connection_test udp 1 3 8000
ssh_connection_test udp 3 1 8000
ssh_connection_test udp 3 2 8000
ssh_connection_test udp 2 3 8000
ssh_no_connection_test tcp 1 2 8000
ssh_no_connection_test tcp 2 1 8000

delete_sg sg-1 0
ssh_no_connection_test tcp 1 2 8000
ssh_no_connection_test tcp 2 1 8000
ssh_no_connection_test tcp 1 3 8000
ssh_no_connection_test tcp 3 1 8000
ssh_no_connection_test tcp 3 2 8000
ssh_no_connection_test tcp 2 3 8000
ssh_no_connection_test udp 1 2 8000
ssh_no_connection_test udp 2 1 8000

add_sg sg-1 0 1
add_sg_rule_port_open tcp sg-1 0 1 9000
ssh_connection_test tcp 1 2 9000
ssh_connection_test tcp 2 1 9000
ssh_connection_test tcp 1 3 9000
ssh_connection_test tcp 3 1 9000
ssh_connection_test tcp 3 2 9000
ssh_connection_test tcp 2 3 9000
ssh_no_connection_test udp 1 2 9000
ssh_no_connection_test udp 2 1 9000
qemu_stop 1
qemu_stop 2
qemu_stop 3
server_stop 0
network_disconnect 0 1
return_result

