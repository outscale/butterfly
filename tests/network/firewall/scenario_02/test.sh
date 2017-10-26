#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42 sg-1
nic_add 0 2 42 sg-1
nic_add 0 3 42 sg-2
qemus_start 1 2 3

sg_member_add 0 sg-1 42.0.0.1
sg_member_add 0 sg-1 42.0.0.2
sg_member_add 0 sg-2 42.0.0.3
sg_rule_add_with_sg_member udp sg-1 0 8000 sg-2
ssh_connection_test udp 3 1 8000
ssh_connection_test udp 3 2 8000
ssh_no_connection_test udp 1 3 8000
ssh_no_connection_test udp 1 3 8000
ssh_no_connection_test udp 1 2 8000
ssh_no_connection_test udp 2 1 8000

sg_rule_add_with_sg_member udp sg-1 0 8000 sg-1
ssh_connection_test udp 3 1 8000
ssh_connection_test udp 3 2 8000
ssh_no_connection_test udp 1 3 8000
ssh_no_connection_test udp 2 3 8000
ssh_connection_test udp 1 2 8000
ssh_connection_test udp 2 1 8000

remove_sg_from_nic 0 2
sg_member_del 0 sg-1 42.0.0.2
ssh_connection_test udp 3 1 8000
ssh_no_connection_test udp 3 2 8000
ssh_no_connection_test udp 1 3 8000
ssh_no_connection_test udp 2 3 8000
ssh_no_connection_test udp 1 2 8000
ssh_no_connection_test udp 2 1 8000

qemus_stop 1 2 3
server_stop 0
network_disconnect 0 1
return_result
