#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $(dirname $0)/../functions.sh

network_connect 0 1
server_start 0
server_start 1
nic_add 0 1 42 sg-1
nic_add 0 2 42 sg-1
nic_add 1 3 42 sg-1
nic_add 1 4 42 sg-1
sg_rule_add_all_open 0 sg-1
sg_rule_add_all_open 1 sg-1
qemu_start 1
qemu_start 2
qemu_start 3
qemu_start 4
ssh_connection_test tcp 1 2 1234
ssh_connection_test tcp 1 3 5554
ssh_connection_test tcp 1 4 4123
ssh_connection_test tcp 2 1 3234
ssh_connection_test tcp 2 3 5554
ssh_connection_test tcp 2 4 4123
ssh_connection_test tcp 3 1 1234
ssh_connection_test tcp 3 2 5554
ssh_connection_test tcp 3 4 4123
ssh_connection_test tcp 4 1 5004
ssh_connection_test tcp 4 2 5964
ssh_connection_test tcp 4 3 5554
qemu_stop 1
qemu_stop 2
qemu_stop 3
qemu_stop 4
server_stop 0
server_stop 1
network_disconnect 0 1
return_result

