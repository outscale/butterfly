#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add 0 1 42 sg-1
nic_add_noip 0 2 42 sg-1
sg_rule_add_all_open 0 sg-1
qemus_start 1 2

for i in $(seq 1 10); do
    ssh_no_ping 1 2
    nic_update_ip 0 2 42.0.0.2
    sleep 1
    ssh_ping 1 2
    nic_update_ip 0 2
    sleep 1
done

qemus_stop 2 1
server_stop 0
network_disconnect 0 1
return_result

