#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
nic_add 0 1 42 sg-1
nic_add 0 2 1337 sg-1
nic_add 1 3 1337 sg-1
nic_add 1 4 42 sg-1
sg_rule_add_all_open 0 sg-1
sg_rule_add_all_open 1 sg-1
qemu_start_async 1
qemu_start_async 2
qemu_start_async 3
qemu_start_async 4
qemus_wait 1 2 3 4
ssh_no_ping 1 2
ssh_no_ping 1 3
ssh_ping 1 4
ssh_ping 4 1
ssh_no_ping 4 2
ssh_no_ping 4 3
qemu_stop 1
qemu_stop 2
qemu_stop 3
qemu_stop 4
server_stop 0
server_stop 1
network_disconnect 0 1
return_result

