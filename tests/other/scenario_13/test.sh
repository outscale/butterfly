#!/bin/bash

#connect 2 qemu using taps together on the a diferent butterfly, and test connection

# [tap 1] <----> [Butterfly 0] <-------> [Butterfly 1] <----> [tap 2]

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
tap_create 0 1 42 sg-1
tap_create 1 2 42 sg-1
tap_create 0 3 42 sg-1
sg_rule_add_all_open 0 sg-1
sg_rule_add_all_open 1 sg-1

qemu_start 1 "-" "tap"
qemu_start 2 "-" "tap"
qemu_start 3 "-" "tap"

ssh_ping 1 3
ssh_ping 1 2
ssh_iperf3_tcp 1 3
ssh_iperf3_tcp 1 2
ssh_iperf3_udp 1 2

network_disconnect 0 1

qemu_stop 1
qemu_stop 2
qemu_stop 3

server_stop 0
server_stop 1

return_result
