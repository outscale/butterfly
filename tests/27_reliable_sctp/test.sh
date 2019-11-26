#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
CUR_TEST="$(dirname $0)"
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
nic_add 0 1 42 sg-1
nic_add 1 2 42 sg-1
qemus_start 1 2

ssh_no_connection_test sctp 1 2 8000
ssh_no_connection_test sctp 2 1 8000

sg_rule_add_all_open 0 sg-1
sg_rule_add_all_open 1 sg-1
ssh_connection_test sctp 1 2 8000
ssh_connection_test sctp 2 1 8000

sg_rule_del_all_open 0 sg-1
sg_rule_del_all_open 1 sg-1
ssh_no_connection_test sctp 1 2 8000
ssh_no_connection_test sctp 2 1 8000

cli 0 0 sg rule add $sg --dir in --ip-proto 132 --cidr 0.0.0.0/0
cli 1 0 sg rule add $sg --dir in --ip-proto 132 --cidr 0.0.0.0/0
ssh_connection_test sctp 1 2 8000
ssh_connection_test sctp 2 1 8000

qemus_stop 1 2
server_stop 0
server_stop 1
network_disconnect 0 1
return_result
