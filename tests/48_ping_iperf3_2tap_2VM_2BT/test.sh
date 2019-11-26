#!/bin/bash

#connect 2 taps together on the a diferent butterfly, and test connection

# [tap 1] <----> [Butterfly 0] <-------> [Butterfly 1] <----> [tap 2]

CUR_TEST="$(dirname $0)"
BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
tap_add 0 1 42 sg-1
tap_add 1 2 42 sg-1
tap_add 0 3 42 sg-1
sg_rule_add_all_open 0 sg-1
sg_rule_add_all_open 1 sg-1

tap_ping 1 2
tap_ping 2 1
tap_ping 1 3

tap_iperf3_tcp 1 2
tap_iperf3_tcp 2 1
tap_iperf3_tcp 1 3

network_disconnect 0 1
server_stop 0
server_stop 1
tap_del 1
tap_del 2
tap_del 3
return_result
