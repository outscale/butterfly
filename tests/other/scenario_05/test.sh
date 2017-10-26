#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
for i in `seq 0 5`; do
    if [ "$i" -le "3" ]; then
        echo "===== net_Bonding mode $i ===="
        server_start_bonding 0 $i
        server_start_bonding 1 $i
        nic_add 0 1 42 sg-1
        nic_add 1 2 42 sg-1
        sg_rule_add_all_open 0 sg-1
        sg_rule_add_all_open 1 sg-1
        qemu_start_async 1
        qemu_start_async 2
        qemus_wait 1 2
        ssh_ping 1 2
        ssh_ping 2 1
        ssh_connection_test tcp 1 2 4550
        ssh_connection_test tcp 2 1 8500
        ssh_connection_test udp 1 2 7543
        ssh_connection_test udp 2 1 6041
        qemu_stop 1
        qemu_stop 2
        server_stop 0
        server_stop 1
    else
        echo "net_bonding mode $i doesn't work yet"
    fi
done
network_disconnect 0 1
return_result

