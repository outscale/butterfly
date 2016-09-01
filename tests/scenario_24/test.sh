#!/bin/bash

BUTTERFLY_SRC_ROOT=$1
BUTTERFLY_BUILD_ROOT=$2

source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0
nic_add sg-1 0 1 42
nic_add sg-1 0 2 42
qemu_start 1
qemu_start 2
for i in {1..50}; do
    ssh_connection_test udp 1 2 1234
    ret=$?
    if [ $ret == 1 ]; then
	qemu_stop 1
	qemu_stop 2
	server_stop 0
	network_disconnect 0 1
	return_result
    fi
    ssh_connection_test udp 2 1 1234
    ret=$?
    if [ $ret == 1 ]; then
	qemu_stop 1
	qemu_stop 2
	server_stop 0
	network_disconnect 0 1
	return_result
    fi
done

for i in {1..50}; do
    ssh_connection_test tcp 1 2 1234
    ret=$?
    if [ $ret == 1 ]; then
	qemu_stop 1
	qemu_stop 2
	server_stop 0
	network_disconnect 0 1
	return_result
    fi
    ssh_connection_test tcp 2 1 1234
    ret=$?
    if [ $ret == 1 ]; then
	qemu_stop 1
	qemu_stop 2
	server_stop 0
	network_disconnect 0 1
	return_result
    fi
done
qemu_stop 1
qemu_stop 2
server_stop 0
network_disconnect 0 1
return_result

