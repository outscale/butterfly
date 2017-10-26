#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42 sg-1
nic_add 0 2 42 sg-1
qemus_start 1 2

for i in {1..10}; do
    ssh_no_connection_test udp 1 2 4054
    ssh_no_connection_test udp 2 1 4504
    ssh_no_connection_test tcp 1 2 4450
    ssh_no_connection_test tcp 2 1 4404

    sg_rule_add_all_open 0 sg-1
    ssh_connection_test udp 1 2 4054
    ssh_connection_test udp 2 1 4504
    ssh_connection_test tcp 1 2 4450
    ssh_connection_test tcp 2 1 4404

    sg_add sg-1 0
done
qemu_stop 1 2
server_stop 0
network_disconnect 0 1
return_result
