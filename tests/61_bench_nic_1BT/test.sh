#!/bin/bash

#connect 2 taps together on the same butterfly, and test connection

#
#             / ---> [ Bench snd ]
# [Butterfly 0]
#             \ ---> [ Bench Rcv ]

CUR_TEST="$(dirname $0)"
BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1

server_start 0
sg_rule_add_icmp 0 sg-1

bench_lat_rcv_add 0 2 42 sg-1
bench_lat_snd_add 0 1 2 42 sg-1

sleep 0.5

server_stop 0
network_disconnect 0 1

check_bench_ping 13 15

return_result

