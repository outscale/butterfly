#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42 sg-1
nic_add 0 2 42 sg-1
sg_rule_add_port_open udp 0 5554 sg-1
qemu_start_async 1
qemu_start_async 2
qemus_wait 1 2
ssh_connection_test udp 1 2 5554
ssh_connection_test udp 2 1 5554
ssh_no_connection_test tcp 2 1 4445
ssh_no_connection_test tcp 2 1 5554
ssh_no_connection_test tcp 1 2 1002
ssh_no_connection_test udp 2 1 1234
ssh_no_connection_test tcp 1 2 5895
ssh_no_connection_test udp 2 1 5553
ssh_no_connection_test udp 1 2 5555
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result
