#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add_noip 0 1 42 sg-1
nic_add_noip 0 2 42 sg-1
sg_rule_add_all_open 0 sg-1
qemu_start_async 1 noip
qemu_start_async 2 noip
qemus_wait 1 2

a1=2001:db8:2000:aff0::1
a2=2001:db8:2000:aff0::2
a3=2001:db8:2000:aff0::3
b1=2001:db8:2000:aff0::4
b2=2001:db8:2000:aff0::5
b3=2001:db8:2000:aff0::6

qemu_add_ipv6 1 $a1/64 $a2/64 $a3/64
nic_update 0 1 --ip $a1 --ip $a2 --ip $a3
nic_update 0 2 --ip $b1 --ip $b2 --ip $b3
qemu_add_ipv6 2 $b1/64 $b2/64 $b3/64
sleep 1

ssh_ping_ip6 1 $a1 $b1
ssh_ping_ip6 1 $a1 $b2
ssh_ping_ip6 1 $a1 $b3
ssh_ping_ip6 1 $a2 $b1
ssh_ping_ip6 1 $a2 $b2
ssh_ping_ip6 1 $a2 $b3
ssh_ping_ip6 1 $a3 $b1
ssh_ping_ip6 1 $a3 $b2
ssh_ping_ip6 1 $a3 $b3

ssh_ping_ip6 2 $b1 $a1
ssh_ping_ip6 2 $b1 $a2
ssh_ping_ip6 2 $b1 $a3
ssh_ping_ip6 2 $b2 $a1
ssh_ping_ip6 2 $b2 $a2
ssh_ping_ip6 2 $b2 $a3
ssh_ping_ip6 2 $b3 $a1
ssh_ping_ip6 2 $b3 $a2
ssh_ping_ip6 2 $b3 $a3

qemus_stop 1 2
server_stop 0
network_disconnect 0 1
return_result

