#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add_void 0 1 42
nic_add sg-1 0 2 42
qemu_start 1
qemu_start 2
ssh_connection_test udp 2 1 1243
ssh_connection_test tcp 2 1 5543
ssh_no_connection_test udp 1 2 1234
ssh_no_connection_test tcp 1 2 1254
set_nic_sg sg-1 0 1
ssh_connection_test udp 2 1 1243
ssh_connection_test tcp 2 1 5543
ssh_connection_test udp 1 2 1234
ssh_connection_test tcp 1 2 1354

delete_sg sg-1 0
ssh_no_connection_test tcp 2 1 1903
ssh_no_connection_test udp 2 1 5043
ssh_no_connection_test tcp 1 2 1043
ssh_no_connection_test udp 1 2 5503

sg_rule_add_full_open sg-1 0
ssh_connection_test udp 2 1 1243
ssh_connection_test tcp 2 1 5543
ssh_connection_test udp 1 2 1234
ssh_connection_test tcp 1 2 1354
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result

