#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add_port_open udp sg-1 0 1 42 6000
nic_add_port_open udp sg-1 0 2 42 6000
qemu_start 1
qemu_start 2

for i in {1..10}; do
    delete_sg sg-1 0
    ssh_no_connection_test udp 1 2 6000
    ssh_no_connection_test udp 2 1 6000

    sg_rule_add_port_open udp sg-1 0 6000
    ssh_connection_test udp 1 2 6000
    ssh_connection_test udp 2 1 6000

    delete_sg sg-1 0
    ssh_no_connection_test tcp 1 2 6000
    ssh_no_connection_test tcp 2 1 6000

    sg_rule_add_port_open tcp sg-1 0 6000
    ssh_connection_test tcp 1 2 6000
    ssh_connection_test tcp 2 1 6000
done

qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result
