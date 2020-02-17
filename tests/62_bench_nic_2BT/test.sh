#!/bin/bash

#connect 2 taps together on 2 not same butterfly, and test connection

# [ bench snd ] <--> [Butterfly 0] <--> [ Bench Rcv ]

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1

server_start 0
server_start 1
sg_rule_add_icmp 0 sg-1
sg_rule_add_icmp 1 sg-1

bench_lat_rcv_add 1 2 42 sg-1
bench_lat_snd_add 0 1 2 42 sg-1

sleep 1

network_disconnect 0 1

server_stop 0
server_stop 1

return_result
