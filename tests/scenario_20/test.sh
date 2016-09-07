#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add_no_rules sg-1 0 1 42
nic_add_no_rules sg-1 0 2 42
nic_add_no_rules sg-1 0 3 42
qemu_start 1
qemu_start 2
qemu_start 3

sg_rule_add_port_open udp sg-1 0 8000
ssh_connection_test udp 1 3 8000
is_ssh_test_failed $? 1 3
ssh_connection_test udp 2 3 8000
is_ssh_test_failed $? 1 3

sg_rule_del_port_open udp sg-1 0 8000
ssh_no_connection_test udp 1 3 8000
is_ssh_test_failed $? 1 3
ssh_no_connection_test udp 2 3 8000
is_ssh_test_failed $? 1 3

sg_rule_add_port_open udp sg-1 0 9000
ssh_connection_test udp 1 3 9000
is_ssh_test_failed $? 1 3
ssh_connection_test udp 2 3 9000
is_ssh_test_failed $? 1 3

sg_rule_del_port_open udp sg-1 0 9000
sg_rule_add_port_open tcp sg-1 0 8000
ssh_connection_test tcp 1 3 8000
is_ssh_test_failed $? 1 3
ssh_connection_test tcp 2 3 8000
is_ssh_test_failed $? 1 3

sg_rule_del_port_open tcp sg-1 0 8000
ssh_no_connection_test tcp 1 3 8000
is_ssh_test_failed $? 1 3
ssh_no_connection_test tcp 2 3 8000
is_ssh_test_failed $? 1 3

sg_rule_add_port_open tcp sg-1 0 9000
ssh_connection_test tcp 1 3 9000
is_ssh_test_failed $? 1 3
ssh_connection_test tcp 2 3 9000
is_ssh_test_failed $? 1 3

qemu_stop 1
qemu_stop 2
qemu_stop 3
server_stop 0
network_disconnect 0 1
return_result

