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
update_sg_rules_full_open 0 1 "sg-1"
ssh_udp 2 1 1243
ssh_tcp 2 1 5543
ssh_udp 1 2 1234
ssh_tcp 1 2 1354
delete_nic 0 2
delete_nic 0 1
qemu_stop 1
qemu_stop 2

add_nic_no_sg 0 1 42
add_nic_no_sg 0 2 42
qemu_start 1
qemu_start 2
ssh_no_udp 2 1 1903
ssh_no_tcp 2 1 5043
ssh_no_udp 1 2 1043
ssh_no_tcp 1 2 5503
update_sg_rules_full_open 0 1 "sg-1"
update_sg_rules_full_open 0 2 "sg-1"
sleep 1
ssh_udp 2 1 1243
ssh_tcp 2 1 5543
ssh_udp 1 2 1234
ssh_tcp 1 2 1354
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result

