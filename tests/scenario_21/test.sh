#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
add_nic_void 0 1 42
add_nic_void 0 2 42
qemu_start 1
qemu_start 2

ssh_no_connection_test tcp 1 2 4454    
ssh_no_connection_test tcp 2 1 4454
add_sg sg-1 0 1
add_sg sg-1 0 2

for i in {1..10}; do      
    ssh_no_connection_test tcp 1 2 4454
    ssh_no_connection_test tcp 2 1 4454

    add_sg_rule_port_open tcp sg-1 0 2 4454
    sleep 1
    ssh_connection_test tcp 1 2 4454
    ssh_connection_test tcp 2 1 4454

    delete_rule_port_open tcp sg-1 0 2 4454
    sleep 1
    ssh_no_connection_test tcp 1 2 4454
    ssh_no_connection_test tcp 2 1 4454

    add_sg_rule_port_open udp sg-1 0 2 5554
    sleep 1
    ssh_connection_test udp 1 2 5554
    ssh_connection_test udp 2 1 5554

    delete_sg sg-1 0 1
    sleep 1
    ssh_no_connection_test tcp 1 2 4454
    ssh_no_connection_test tcp 2 1 4454
    ssh_no_connection_test udp 1 2 5554
    ssh_no_connection_test udp 2 1 5554
done

qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result
