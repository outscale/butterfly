#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

# Add a two nics on the same butterfly on the same VNI and start VMs
server_start 0
client_add_nic 0 1 666
client_add_nic 0 2 666
qemu_start 1
qemu_start 2
sleep 3
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
