#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42 sg-1
nic_add 0 2 42 sg-1
sg_rule_add_all_open 0 sg-1
qemu_start 1
qemu_start 2
ssh_connection_test udp 2 1 1234
ssh_connection_test udp 1 2 1234
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result

