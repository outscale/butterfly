#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add_no_rules sg-1 0 1 42
nic_add_no_rules sg-1 0 2 42
qemu_start 1
qemu_start 2
ssh_no_connection_test udp 2 1 5554
ssh_no_connection_test udp 1 2 5554
ssh_no_connection_test tcp 1 2 4445
ssh_no_connection_test tcp 2 1 4445

sg_rule_add_full_open sg-1 0
ssh_connection_test udp 1 2 4445
ssh_connection_test udp 2 1 5554
ssh_connection_test tcp 2 1 5554
ssh_connection_test tcp 1 2 3485
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result

