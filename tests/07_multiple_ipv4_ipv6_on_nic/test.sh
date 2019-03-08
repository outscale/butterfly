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

a1=42.0.0.1
a2=42.0.0.2
b1=42.0.0.3
b2=42.0.0.4
A1=2001:db8:2000:aff0::1
A2=2001:db8:2000:aff0::2
B1=2001:db8:2000:aff0::3
B2=2001:db8:2000:aff0::4

qemu_add_ipv4 1 $a1/24 $a2/24
qemu_add_ipv6 1 $A1/64 $A2/64
nic_update 0 1 --ip $a1 --ip $a2 --ip $A1 --ip $A2
nic_update 0 2 --ip $b1 --ip $b2 --ip $B1 --ip $B2
qemu_add_ipv4 2 $b1/24 $b2/24
qemu_add_ipv6 2 $B1/64 $B2/64
sleep 1

ssh_ping_ip 1 $a1 $b1
ssh_ping_ip 1 $a1 $b2
ssh_ping_ip 1 $a2 $b1
ssh_ping_ip 1 $a2 $b2

ssh_ping_ip6 1 $A1 $B1
ssh_ping_ip6 1 $A1 $B2
ssh_ping_ip6 1 $A2 $B1
ssh_ping_ip6 1 $A2 $B2

qemus_stop 1 2
server_stop 0
network_disconnect 0 1
return_result

