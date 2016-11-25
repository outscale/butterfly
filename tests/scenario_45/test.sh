#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1

# Case 1: no trace option, no filtering bypass
server_start_options 0
server_start_options 1
nic_add 0 1 42 sg-1
nic_add 1 2 42 sg-1
qemu_start 1
qemu_start 2
sg_rule_add_icmp 0 sg-1
sg_rule_add_icmp 1 sg-1
ssh_ping 1 2
nic_add 0 3 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
ssh_ping 1 2
nic_add 0 3 42 sg-1
nic_add 0 4 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
nic_del 0 4
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1

# Case 2: no trace option, filtering bypass
server_start_options 0 
server_start_options 1
nic_add_bypass 0 1 42
nic_add_bypass 1 2 42
qemu_start 1
qemu_start 2
ssh_ping 1 2
nic_add_bypass 0 3 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
ssh_ping 1 2
nic_add_bypass 0 3 42
nic_add_bypass 0 4 42
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
nic_del 0 4
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1

# Case 3: trace option, no filtering bypass
server_start_options 0 -t
server_start_options 1 -t
nic_add 0 1 42 sg-1
nic_add 1 2 42 sg-1
qemu_start 1
qemu_start 2
sg_rule_add_icmp 0 sg-1
sg_rule_add_icmp 1 sg-1
ssh_ping 1 2
nic_add 0 3 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
ssh_ping 1 2
nic_add 0 3 42 sg-1
nic_add 0 4 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
nic_del 0 4
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1

# Case 4: trace option, filtering bypass
server_start_options 0 -t
server_start_options 1 -t
nic_add_bypass 0 1 42
nic_add_bypass 1 2 42
qemu_start 1
qemu_start 2
ssh_ping 1 2
nic_add_bypass 0 3 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
ssh_ping 1 2
nic_add_bypass 0 3 42
nic_add_bypass 0 4 42
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
nic_del 0 4
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1

# Case 5.1 no trace option, mixed filtering bypass scenario
server_start_options 0
server_start_options 1
nic_add 0 1 42 sg-1
nic_add_bypass 1 2 42
qemu_start 1
qemu_start 2
sg_rule_add_icmp 0 sg-1
sg_rule_add_icmp 1 sg-1
ssh_ping 1 2
nic_add 0 3 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
ssh_ping 1 2
nic_add 0 3 42 sg-1
nic_add 0 4 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
nic_del 0 4
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1

# Case 5.2 no trace option, mixed filtering bypass scenario
server_start_options 0
server_start_options 1
nic_add 0 1 42 sg-1
nic_add 1 2 42 sg-1
qemu_start 1
qemu_start 2
sg_rule_add_icmp 0 sg-1
sg_rule_add_icmp 1 sg-1
ssh_ping 1 2
nic_add_bypass 0 3 42
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
ssh_ping 1 2
nic_add_bypass 0 3 42
nic_add_bypass 0 4 42
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
nic_del 0 4
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1

# Case 6.1 trace option, mixed filtering bypass scenario
server_start_options 0 -t
server_start_options 1 -t
nic_add 0 1 42 sg-1
nic_add_bypass 1 2 42
qemu_start 1
qemu_start 2
sg_rule_add_icmp 0 sg-1
sg_rule_add_icmp 1 sg-1
ssh_ping 1 2
nic_add 0 3 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
ssh_ping 1 2
nic_add 0 3 42 sg-1
nic_add 0 4 42 sg-1
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
nic_del 0 4
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1

# Case 6.2 trace option, mixed filtering bypass scenario
server_start_options 0 -t
server_start_options 1 -t
nic_add 0 1 42 sg-1
nic_add 1 2 42 sg-1
qemu_start 1
qemu_start 2
sg_rule_add_icmp 0 sg-1
sg_rule_add_icmp 1 sg-1
ssh_ping 1 2
nic_add_bypass 0 3 42
qemu_start 3
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
ssh_ping 1 2
nic_del 0 3
ssh_ping 1 2
nic_add_bypass 0 3 42
nic_add_bypass 0 4 42
qemu_start 3
ssh_ping 1 2
ssh_ping 1 3
ssh_ping 3 2
qemu_stop 3
nic_del 0 3
nic_del 0 4
ssh_ping 1 2
qemu_stop 1
qemu_stop 2
server_stop 0
server_stop 1

network_disconnect 0 1
return_result

