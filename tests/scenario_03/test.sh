#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
nic_add sg-1 0 1 42
nic_add sg-1 0 2 1337
nic_add sg-1 1 3 1337
nic_add sg-1 1 4 42
qemu_start 1
qemu_start 2
qemu_start 3
qemu_start 4
ssh_no_ping 1 2
ssh_no_ping 1 3
ssh_ping 1 4
ssh_ping 4 1
ssh_no_ping 4 2
ssh_no_ping 4 3
qemu_stop 1
qemu_stop 2
qemu_stop 3
qemu_stop 4
server_stop 0
server_stop 1
network_disconnect 0 1
return_result

