#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
cli 0 0 nic add --ip 10.0.0.1 --mac 01:02:03:04:05:06 --vni 1 --id nic01
cli 0 0 nic add --ip 10.0.0.1 --mac 01:02:03:04:05:06 --vni 2 --id nic02 --enable-antispoof
cli 0 0 nic add --ip 10.0.0.1 --mac 01:02:03:04:05:06 --vni 3 --id nic03 --bypass-filtering
cli 0 0 nic add --ip 10.0.0.1 --mac 01:02:03:04:05:06 --vni 1 --id nic01 --sg sg-1 --sg sg-2 --packet-trace true --trace-path test.pcap
cli 0 1 nic add --id nic04 --ip 10.0.0.1 --mac 01:02:03:04:05:06 --type 0 --vni 2 --sg sg-1 --enable-antispoof true --trace-path ./test.pcap
cli 0 1 nic add --id nic05 --ip 10.0.0.1 --mac 01:02:03:04:05:06 --type TESTS + nic update --id nic02 --packet-trace false --trace-path ./tests.pcap
cli 0 0 nic update --id nic01 --enable-antispoof true --packet-trace true + nic update --id nic01 --enable-antispoof false + nic update --id nic01 --enable-antispoof true + --version + -V
cli 0 1 nic update --id nic01 --trace-path ./tests.pcap + nic update

cli 0 0 nic add --ip 10.0.0.1 --mac 01:02:03:04:05:06 --vni 4 --id nic04
cli 0 0 nic add --ip 10.0.0.2 --mac 02:02:03:04:05:06 --vni 4 --id nic05
cli 0 0 nic add --ip 10.0.0.3 --mac 03:02:03:04:05:06 --vni 4 --id nic06
cli 0 0 nic add --ip 10.0.0.1 --mac 01:02:03:04:05:06 --vni 5 --id nic07 --enable-antispoof
cli 0 0 nic add --ip 10.0.0.2 --mac 02:02:03:04:05:06 --vni 5 --id nic08 --enable-antispoof
cli 0 0 nic add --ip 10.0.0.3 --mac 03:02:03:04:05:06 --vni 5 --id nic09 --enable-antispoof
cli 0 0 nic add --ip 10.0.0.1 --mac 01:02:03:04:05:06 --vni 6 --id nic10 --bypass-filtering
cli 0 0 nic add --ip 10.0.0.2 --mac 02:02:03:04:05:06 --vni 6 --id nic11 --bypass-filtering
cli 0 0 nic add --ip 10.0.0.3 --mac 03:02:03:04:05:06 --vni 6 --id nic12 --bypass-filtering

cli 0 0 sg add sg-a sg-b sg-c sg-d sg-e sg-f

cli 0 0 nic sg add nic01 sg-a
cli 0 0 sg member add sg-a 10.0.0.1

cli 0 0 nic sg add nic02 sg-b
cli 0 0 sg member add sg-b 10.0.0.1

cli 0 0 nic sg add nic03 sg-c
cli 0 0 sg member add sg-c 10.0.0.1

cli 0 0 nic sg add nic04 sg-d
cli 0 0 nic sg add nic05 sg-d
cli 0 0 nic sg add nic06 sg-d
cli 0 0 sg member add sg-d 10.0.0.1
cli 0 0 sg member add sg-d 10.0.0.2
cli 0 0 sg member add sg-d 10.0.0.3

cli 0 0 nic sg add nic07 sg-e
cli 0 0 nic sg add nic08 sg-e
cli 0 0 nic sg add nic09 sg-e
cli 0 0 sg member add sg-e 10.0.0.1
cli 0 0 sg member add sg-e 10.0.0.2
cli 0 0 sg member add sg-e 10.0.0.3

cli 0 0 nic sg add nic10 sg-f
cli 0 0 nic sg add nic11 sg-f
cli 0 0 nic sg add nic12 sg-f
cli 0 0 sg member add sg-f 10.0.0.1
cli 0 0 sg member add sg-f 10.0.0.2
cli 0 0 sg member add sg-f 10.0.0.3

cli 0 0 sg rule add sg-a --ip-proto tcp --port-start 22 --port-end 80 --cidr 42.0.0.1/32
cli 0 0 sg rule add sg-b --ip-proto all --cidr 0.0.0.0/0
cli 0 0 sg rule add sg-c --ip-proto udp --port 8000 --cidr 0.0.0.0/0
cli 0 0 sg rule add sg-c --ip-proto udp --port 8080 --cidr 0.0.0.0/0
cli 0 0 sg rule add sg-c --ip-proto udp --port 8888 --cidr 0.0.0.0/0
cli 0 0 sg rule add sg-d --ip-proto icmp --sg-members sg-d
cli 0 0 sg rule add sg-d --ip-proto all --sg-members sg-d

cli 0 0 dump
cp $BUTTERFLY_BUILD_ROOT/cli_output $BUTTERFLY_BUILD_ROOT/dump1.req
server_stop 0
server_start 0
cli 0 0 request $BUTTERFLY_BUILD_ROOT/dump1.req
cli 0 0 dump
cp $BUTTERFLY_BUILD_ROOT/cli_output $BUTTERFLY_BUILD_ROOT/dump2.req
if ! diff $BUTTERFLY_BUILD_ROOT/dump1.req $BUTTERFLY_BUILD_ROOT/dump2.req ; then
    fail "double dump test failed"
fi

$BUTTERFLY_BUILD_ROOT/api/client/butterfly
cli 0 1
cli 0 0 -h
cli 0 0 --help
cli 0 1 -k
$BUTTERFLY_BUILD_ROOT/api/client/butterfly -v
$BUTTERFLY_BUILD_ROOT/api/client/butterfly --version

$BUTTERFLY_BUILD_ROOT/api/client/butterfly nic stats
$BUTTERFLY_BUILD_ROOT/api/client/butterfly nic details
$BUTTERFLY_BUILD_ROOT/api/client/butterfly nic sg list
cli 0 0 nic stats help
cli 0 0 nic details help
cli 0 0 nic sg list help
cli 0 1 nic sg list -v

cli 0 1 nic details request
cli 0 1 nic stats request
cli 0 0 nic list help
cli 0 0 nic list
cli 0 0 status

cli 0 1 request
cli 0 0 shutdown
cli 0 0 dump help

server_stop 0
network_disconnect 0 1
return_result

