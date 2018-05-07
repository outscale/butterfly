#!/bin/bash

#connect 2 taps together on the same butterfly, and test connection

#
#             / ---> [ tap 1 ]
# [Butterfly 0]
#             \ ---> [ tap 2 ]

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
tap_add 0 1 42 sg-1
tap_add 0 2 42 sg-1
sg_rule_add_all_open 0 sg-1

sudo ip netns exec ns1 ping 42.0.0.2 -c 2
sudo ip netns exec ns2 ping 42.0.0.1 -c 2

sudo ip netns exec ns1 iperf3 -s &
sleep 1
sudo ip netns exec ns2 iperf3 -c 42.0.0.1 -t 5

network_disconnect 0 1
server_stop 0
tap_del 1
tap_del 2
return_result
