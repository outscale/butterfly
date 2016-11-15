#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $(dirname $0)/../functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42
nic_add 0 2 42 sg-1
sg_rule_add_all_open 0 sg-1
qemu_start 1
qemu_start 2
ssh_connection_test udp 1 2 5543
ssh_connection_test tcp 1 2 1243
ssh_no_connection_test udp 2 1 1254
ssh_no_connection_test tcp 2 1 1234
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result

