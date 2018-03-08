#!/bin/bash

#check that dperf work

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh
source $BUTTERFLY_SRC_ROOT/scripts/error_print.sh

[ -z "$RTE_SDK" ] && error_print "RTE_SDK path is not set" && exit 1

cp -rvf $BUTTERFLY_BUILD_ROOT/3rdparty-build/packetgraph/.libs $RTE_SDK/pg-libs
cp -v $BUTTERFLY_SRC_ROOT/scripts/benchmark-docker-file $RTE_SDK/Dockerfile

network_connect 0 1
server_start_huge 0

# nic_add_bypass 0 1 42 sg-1
# nic_add_bypass 0 2 42 sg-1
# if you want to test without firewall uncomment above lines and comment belove lines
nic_add 0 1 42 sg-1
nic_add 0 2 42 sg-1
sg_rule_add_all_open 0 sg-1

$BUTTERFLY_SRC_ROOT/scripts/docker_bench_build.sh
sleep 1
$BUTTERFLY_SRC_ROOT/scripts/start_dperf_docker.sh &
$BUTTERFLY_SRC_ROOT/scripts/start_dperf_docker_serv.sh &
sleep 21

network_disconnect 0 1
return_result
