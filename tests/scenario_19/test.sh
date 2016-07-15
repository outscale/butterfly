#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
add_nic_port_open 0 1 42 17 8000
add_nic_port_open 0 2 42 17 8000
add_nic_port_open 0 3 42 17 9000
qemu_start 1
qemu_start 2
qemu_start 3
ssh_udp 1 2 8000
ssh_udp 2 1 8000
ssh_udp 1 3 8000
ssh_udp 3 2 8000
ssh_no_tcp 1 2 8000
ssh_no_tcp 2 1 8000
ssh_no_udp 3 1 9000
ssh_no_udp 2 3 9000
qemu_stop 3
delete_nic 0 3

add_nic_port_open 0 3 42 17 8000
qemu_start 3
ssh_udp 1 2 8000
ssh_udp 2 1 8000
ssh_udp 1 3 8000
ssh_udp 3 1 8000
ssh_udp 3 2 8000
ssh_udp 2 3 8000
ssh_no_tcp 1 2 8000
ssh_no_tcp 2 1 8000

delete_nic_sg 0 1 "sg-1"
sleep 1
ssh_no_udp 1 2 8000
ssh_no_udp 2 1 8000
ssh_no_udp 1 3 8000
ssh_no_udp 3 1 8000
ssh_no_udp 3 2 8000
ssh_no_udp 2 3 8000
ssh_no_tcp 1 2 8000
ssh_no_tcp 2 1 8000

update_nic_sg 0 1 "sg-1"
update_sg_rule_port_open 0 1 17 8000 "sg-1" 
sleep 1
ssh_udp 1 2 8000
ssh_udp 2 1 8000
ssh_udp 1 3 8000
ssh_udp 3 1 8000
ssh_udp 3 2 8000
ssh_udp 2 3 8000
ssh_no_tcp 1 2 8000
ssh_no_tcp 2 1 8000

delete_nic_sg 0 1 "sg-1"
sleep 1
ssh_no_tcp 1 2 8000
ssh_no_tcp 2 1 8000
ssh_no_tcp 1 3 8000
ssh_no_tcp 3 1 8000
ssh_no_tcp 3 2 8000
ssh_no_tcp 2 3 8000
ssh_no_udp 1 2 8000
ssh_no_udp 2 1 8000

update_nic_sg 0 1 "sg-1"
update_sg_rule_port_open 0 1 6 9000 "sg-1" 
sleep 2
ssh_tcp 1 2 9000
ssh_tcp 2 1 9000
ssh_tcp 1 3 9000
ssh_tcp 3 1 9000
ssh_tcp 3 2 9000
ssh_tcp 2 3 9000
ssh_no_udp 1 2 9000
ssh_no_udp 2 1 9000
qemu_stop 1
qemu_stop 2
qemu_stop 3
server_stop 0
network_disconnect 0 1
return_result

