#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42
nic_add 0 2 42
qemus_start 1 2

ssh_no_connection_test tcp 2 1 4454
ssh_no_connection_test tcp 1 2 4454
nic_set_sg 0 1 sg-1
nic_set_sg 0 2 sg-1

for i in {1..3}; do
    ssh_no_connection_test tcp 2 1 4454
    ssh_no_connection_test tcp 1 2 4454

    sg_rule_add_port_open tcp 0 4454 sg-1
    ssh_connection_test tcp 2 1 4454
    ssh_connection_test tcp 1 2 4454

    sg_rule_del 0 sg-1 tcp 4454
    ssh_no_connection_test tcp 2 1 4454
    ssh_no_connection_test tcp 1 2 4454

    sg_rule_add_port_open udp 0 5554 sg-1
    ssh_connection_test udp 2 1 5554
    ssh_connection_test udp 1 2 5554

    sg_del 0 sg-1
    ssh_no_connection_test tcp 2 1 4454
    ssh_no_connection_test tcp 1 2 4454
    ssh_no_connection_test udp 2 1 5554
    ssh_no_connection_test udp 1 2 5554
done
qemus_stop 1 2
server_stop 0
network_disconnect 0 1
return_result
