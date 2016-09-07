#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add_port_open tcp sg-1 0 1 42 4454
nic_add_port_open tcp sg-1 0 2 42 4454
qemu_start 1
qemu_start 2
ssh_connection_test tcp 2 1 4454
is_ssh_test_failed $? 1 2
ssh_connection_test tcp 1 2 4454
is_ssh_test_failed $? 1 2
ssh_no_connection_test tcp 1 2 4452
is_ssh_test_failed $? 1 2
ssh_no_connection_test tcp 1 2 4453
is_ssh_test_failed $? 1 2
ssh_no_connection_test udp 2 1 4454
is_ssh_test_failed $? 1 2
ssh_no_connection_test tcp 1 2 1234
is_ssh_test_failed $? 1 2
ssh_no_connection_test udp 2 1 5895
is_ssh_test_failed $? 1 2
ssh_no_connection_test tcp 1 2 5053
is_ssh_test_failed $? 1 2
ssh_no_connection_test udp 2 1 5505
is_ssh_test_failed $? 1 2
qemu_stop 1
qemu_stop 2
sleep 0.2
server_stop 0
network_disconnect 0 1
return_result

