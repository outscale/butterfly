#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42 sg-1
nic_add 0 2 42
sg_rule_add_port_open udp 0 8000 sg-1
qemus_start 1 2

sg_rule_add_port_open udp 0 9000 sg-2
ssh_connection_test udp 2 1 8000
ssh_no_connection_test udp 2 1 9000

nic_set_sg 0 1 sg-1 sg-2
ssh_connection_test udp 2 1 8000
ssh_connection_test udp 2 1 9000

remove_sg_from_nic 0 1
nic_set_sg 0 1 sg-2
ssh_no_connection_test udp 2 1 8000
ssh_connection_test udp 2 1 9000

remove_sg_from_nic 0 1
ssh_no_connection_test udp 2 1 8000
ssh_no_connection_test udp 2 1 9000

qemus_stop 1 2
server_stop 0
network_disconnect 0 1
return_result
