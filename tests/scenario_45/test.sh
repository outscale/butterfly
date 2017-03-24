#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1

# All combinations is too long (2^4 = 16 scenario)
# for big in {\ +,-t+}{nic_add+,nic_add_bypass+}{nic_add+,nic_add_bypass+}{nic_add+,nic_add_bypass+} ; do
# Remove some cases
for big in \
    +nic_add+nic_add+nic_add+ \
    +nic_add+nic_add_bypass+nic_add+ \
    +nic_add_bypass+nic_add_bypass+nic_add_bypass+ \
    -t+nic_add+nic_add+nic_add+ \
    -t+nic_add_bypass+nic_add+nic_add_bypass+ \
    -t+nic_add_bypass+nic_add_bypass+nic_add_bypass+
    do

    _server_start_options=$(echo $big | cut -d '+' -f 1)
    _nic_add_1=$(echo $big | cut -d '+' -f 2)
    _nic_add_2=$(echo $big | cut -d '+' -f 3)
    _nic_add_3=$(echo $big | cut -d '+' -f 4)

    # both servers start with or without trace option
    server_start_options 0 $_server_start_options
    server_start_options 1 $_server_start_options
    nic_add 0 1 42 sg-1
    nic_add 1 2 42 sg-1
    qemus_start 1 2
    sg_rule_add_icmp 0 sg-1
    sg_rule_add_icmp 1 sg-1
    ssh_ping 1 2

    # nic added with or without bypass
    $_nic_add_1 0 3 42 sg-1

    qemu_start 3
    ssh_ping 1 2
    ssh_ping 1 3
    ssh_ping 3 2
    qemu_stop 3
    nic_del 0 3
    ssh_ping 1 2

    # nic added with or without bypass
    $_nic_add_2 0 3 42 sg-1

    # nic added with or without bypass
    $_nic_add_3 0 4 42 sg-1

    qemu_start 3
    ssh_ping 1 2
    ssh_ping 1 3
    ssh_ping 3 2
    qemu_stop 3
    nic_del 0 3
    nic_del 0 4
    ssh_ping 1 2
    qemus_stop 1 2
    server_stop 0
    server_stop 1

done
network_disconnect 0 1
