#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add6 0 1 42 sg-1
nic_add6 0 2 42 sg-1
qemus_start 1 2

ssh_no_ping6 1 2
ssh_no_ping6 2 1

sg_rule_add_ip 0 2001:db8:2000:aff0::1 128 sg-1
sg_rule_add_ip 0 2001:db8:2000:aff0::2 128 sg-1
ssh_ping6 1 2
ssh_ping6 2 1

sg_rule_del_ip 0 2001:db8:2000:aff0::1 128 sg-1
sg_rule_del_ip 0 2001:db8:2000:aff0::2 128 sg-1
ssh_no_ping6 1 2
ssh_no_ping6 2 1

sg_rule_add_icmp6 0 sg-1
ssh_ping6 1 2
ssh_ping6 2 1

sg_rule_add_ip_and_port tcp 0 2001:db8:2000:aff0::1 128 8000 sg-1
ssh_connection_test tcp6 1 2 8000
ssh_no_connection_test tcp6 2 1 8000

sg_rule_add_ip_and_port udp 0 2001:db8:2000:aff0::1 128 8000 sg-1
ssh_connection_test udp6 1 2 8000
ssh_no_connection_test udp6 2 1 8000

sg_del 0 sg-1
ssh_no_ping6 1 2
ssh_no_ping6 2 1
ssh_no_connection_test tcp6 1 2 8000
ssh_no_connection_test tcp6 2 1 8000
ssh_no_connection_test udp6 1 2 8000
ssh_no_connection_test udp6 2 1 8000

qemus_stop 1 2
server_stop 0
network_disconnect 0 1
return_result
