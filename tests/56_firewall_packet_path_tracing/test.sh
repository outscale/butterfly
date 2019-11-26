#!/bin/bash

CUR_TEST="$(dirname $0)"
BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
server_start 1
nic_add 0 1 42 sg-1
nic_add 1 2 42 sg-1
sg_rule_add_all_open 0 sg-1
sg_rule_add_all_open 1 sg-1
qemu_start_async 1
qemu_start_async 2
qemus_wait 1 2

ssh_ping 1 2
ssh_ping 2 1
ssh_connection_test tcp 1 2 4550
ssh_connection_test tcp 2 1 8500
ssh_connection_test udp 1 2 7543
ssh_connection_test udp 2 1 6041
ssh_connection_test sctp 1 2 7500
ssh_connection_test sctp 2 1 8506
test_packet_tracing true 1 2

nic_update 0 1 --packet-trace true --trace-path ./trace_nic1.pcap
nic_update 1 2 --packet-trace true --trace-path ./trace_nic2.pcap

ssh_ping 1 2
ssh_ping 2 1
ssh_connection_test tcp 1 2 4550
ssh_connection_test tcp 2 1 8500
ssh_connection_test udp 1 2 7543
ssh_connection_test udp 2 1 6041
ssh_connection_test sctp 1 2 7500
ssh_connection_test sctp 2 1 8506
test_packet_trace_path true ./trace_nic1.pcap ./trace_nic2.pcap
nic_update 0 1 --packet-trace false

ssh_ping 1 2
ssh_ping 2 1
ssh_connection_test tcp 1 2 4550
ssh_connection_test tcp 2 1 8500
ssh_connection_test udp 1 2 7543
ssh_connection_test udp 2 1 6041
ssh_connection_test sctp 1 2 7500
ssh_connection_test sctp 2 1 8506
ssh_connection_test sctp 1 2 7800
ssh_connection_test sctp 2 1 9006
test_packet_trace_path true ./trace_nic1.pcap ./trace_nic2.pcap

nic_update 0 1 --packet-trace true --trace-path /tmp/trace_nic1.pcap
nic_update 1 2 --packet-trace true --trace-path /tmp/trace_nic2.pcap

ssh_ping 1 2
ssh_ping 2 1
ssh_connection_test tcp 1 2 4550
ssh_connection_test tcp 2 1 8500
ssh_connection_test udp 1 2 7543
ssh_connection_test udp 2 1 6041
ssh_connection_test sctp 1 2 7500
ssh_connection_test sctp 2 1 8506
test_packet_trace_path true /tmp/trace_nic1.pcap /tmp/trace_nic2.pcap
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1
network_disconnect 0 1
return_result
