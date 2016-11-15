#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $(dirname $0)/../functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42 sg-1
nic_add 0 2 42 sg-1
nic_add 0 3 42 sg-1
qemu_start 1
qemu_start 2
qemu_start 3

ssh_no_connection_test udp 3 1 8000
ssh_no_connection_test udp 3 2 8000

sg_rule_add_port_open udp 0 8000 sg-1
ssh_connection_test udp 3 1 8000
ssh_connection_test udp 3 2 8000

sg_rule_del 0 sg-1 udp 8000
ssh_no_connection_test udp 3 1 8000
ssh_no_connection_test udp 3 2 8000

sg_rule_add_port_open udp 0 9000 sg-1
ssh_connection_test udp 3 1 9000
ssh_connection_test udp 3 2 9000

sg_rule_del 0 sg-1 udp 9000
sg_rule_add_port_open tcp 0 8000 sg-1
ssh_connection_test tcp 3 1 8000
ssh_connection_test tcp 3 2 8000

sg_rule_del 0 sg-1 tcp 8000
ssh_no_connection_test tcp 3 1 8000
ssh_no_connection_test tcp 3 2  8000

sg_rule_add_port_open tcp 0 9000 sg-1
ssh_connection_test tcp 3 1 9000
ssh_connection_test tcp 3 1 9000

qemu_stop 1
qemu_stop 2
qemu_stop 3
server_stop 0
network_disconnect 0 1
return_result
