#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
nic_add 0 1 42 sg-1
nic_add 1 2 42 sg-1
sg_rule_add_dhcp 0 sg-1
sg_rule_add_dhcp 1 sg-1
qemu_start_async 1 dhcp-server
qemu_start_async 2 dhcp-client
qemus_wait 1 2
ssh_no_ping 1 2
ssh_no_ping 2 1
sg_rule_add_icmp 0 sg-1
sg_rule_add_icmp 1 sg-1
ssh_ping 1 2
ssh_ping 2 1
qemus_stop 1 2
server_stop 0
server_stop 1
network_disconnect 0 1
return_result

