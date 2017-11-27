#!/bin/bash

#check that a tap can comunicate with a vhost, on the same or on a diferent tap

# [tap 1] <----> [Butterfly 0] <-------> [Butterfly 1] <----> [nic 2]
# [nic 1] <--/

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
tap_add 0 1 42 sg-1
nic_add 0 2 42 sg-1
nic_add 1 3 42 sg-1
sg_rule_add_all_open 0 sg-1
sg_rule_add_all_open 1 sg-1
qemus_start 2 3

sudo ip netns exec ns1 ping 42.0.0.2 -c 5
ssh_run 2 ping 42.0.0.1 -c 5
ssh_run 3 ping 42.0.0.1 -c 5

sudo ip netns exec ns1 iperf3 -s &
sleep 1
ssh_run 2 iperf3 -c 42.0.0.1 -t 5
ssh_run 3 iperf3 -c 42.0.0.1 -t 5

tap_del 1
qemu_stop 2
server_stop 0
server_stop 1
network_disconnect 0 1
return_result
