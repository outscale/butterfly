#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../../" && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
server_start 0

cli 0 1
cli 0 0 -h
cli 0 0 --help
cli 0 1 badcommand
cli 0 1 --not-a-command

cli 0 1 nic
cli 0 0 nic help

cli 0 1 nic add
cli 0 0 nic add help
cli 0 0 nic add --ip 42.0.0.1 --mac 42:42:42:42:42:01 --vni 1337 --id nic_1
cli 0 0 nic add --ip 42.0.0.1 --mac 42:42:42:42:42:01 --vni 1337 --id nic_1
cli 0 0 nic add --ip 42.0.0.2 --mac 42:42:42:42:42:01 --vni 1337 --id nic_2 --enable-antispoof
cli 0 0 nic add --mac 42:42:42:42:42:01 --vni 1337 --id nic_not_created
cli 0 1 nic add --ip 42.0.0.1 --vni 1337 --id nic_not_created
cli 0 1 nic add --ip 42.0.0.1 --mac 42:42:42:42:42:01 --id nic_not_created
cli 0 1 nic add --ip 42.0.0.1 --mac 42:42:42:42:42:01 --vni 1337

cli 0 0 nic list
cli 0 0 nic list help
cli 0 0 nic list -v
cli 0 0 nic list --verbose

cli 0 1 nic stats
cli 0 0 nic stats help
cli 0 0 nic stats nic_1
cli 0 0 nic stats nic_2
cli 0 1 nic stats nic_not_here

cli 0 1 nic details
cli 0 0 nic details help
cli 0 0 nic details nic_1
cli 0 0 nic details nic_2
cli 0 1 nic details nic_not_here

cli 0 1 nic sg
cli 0 0 nic sg help

cli 0 1 nic sg add
cli 0 0 nic sg add help
cli 0 0 nic sg add nic_1 sg-1
cli 0 0 nic sg add nic_1 sg-1
cli 0 0 nic sg add nic_2 sg-1 sg-2
cli 0 0 nic sg add nic_2 sg-1 sg-2
cli 0 1 nic sg add nic_not_here sg-1
cli 0 1 nic sg add nic_not_here sg-1 sg-2
cli 0 1 nic sg add sg-1
cli 0 1 nic sg add sg-1 sg-2

cli 0 1 nic sg list
cli 0 0 nic sg list help
cli 0 0 nic sg list nic_1
cli 0 0 nic sg list nic_2
cli 0 1 nic sg list nic_not_here

cli 0 1 nic sg del
cli 0 0 nic sg del help
cli 0 0 nic sg del nic_1 sg-1
cli 0 0 nic sg del nic_1 sg-1
cli 0 0 nic sg del nic_1 sg-not-here
cli 0 0 nic sg del nic_2 sg-not-here
cli 0 0 nic sg del nic_2 sg-1 sg-2
cli 0 0 nic sg del nic_2 sg-1 sg-2
cli 0 1 nic sg del nic_not_here sg-1
cli 0 1 nic sg del nic_not_here sg-1 sg-2
cli 0 1 nic sg del sg-1
cli 0 1 nic sg del sg-1 sg-2

cli 0 1 nic del
cli 0 0 nic del help
cli 0 0 nic del nic_1
cli 0 0 nic del nic_1
cli 0 0 nic add --ip 42.0.0.1 --mac 42:42:42:42:42:01 --vni 1337 --id nic_1
cli 0 0 nic del nic_1 nic_2 nic_not_here
cli 0 0 nic add --ip 42.0.0.1 --mac 42:42:42:42:42:01 --vni 1337 --id nic_1
cli 0 0 nic add --ip 42.0.0.2 --mac 42:42:42:42:42:01 --vni 1337 --id nic_2 --enable-antispoof
cli 0 0 nic add --ip 42.0.0.2 --mac 42:42:42:42:42:01 --vni 1337 --id nic_22 --bypass-filtering

cli 0 1 sg
cli 0 0 sg help

cli 0 1 sg add
cli 0 0 sg add help
cli 0 0 sg add sg-3
cli 0 0 sg add sg-1
cli 0 0 sg add sg-1 sg-2 sg-3

cli 0 0 sg list
cli 0 0 sg list help

cli 0 1 sg rule
cli 0 0 sg rule help

cli 0 1 sg rule add
cli 0 0 sg rule add help
cli 0 0 sg rule add sg-1 --dir in --ip-proto tcp --port 22 --cidr 10.0.0.0/24
cli 0 0 sg rule add sg-1 --dir in --ip-proto tcp --port 22 --cidr 10.0.0.0/24
cli 0 0 sg rule add sg-1 --ip-proto 42 --cidr 10.0.0.0/24
cli 0 0 sg rule add sg-1 --dir in --ip-proto tcp --port-start 1000 --port-end 2000 --cidr 10.0.0.0/24
cli 0 0 sg rule add sg-1 --ip-proto 42 --sg-members sg-2
cli 0 0 sg rule add sg-1 --ip-proto 42 --sg-members sg-2
cli 0 0 sg rule add sg-1 --ip-proto tcp --port 9999 --sg-members sg-2
cli 0 0 sg rule add sg-1 --ip-proto udp --port 9999 --sg-members sg-2
cli 0 0 sg rule add sg-1 --ip-proto icmp --sg-members sg-2
cli 0 0 sg rule add sg-1 --ip-proto 42 --sg-members sg-3
cli 0 0 sg rule add sg-4 --ip-proto 42 --cidr 10.0.0.0/24
cli 0 0 sg rule add sg-1 --ip-proto 42 --sg-members sg-5
cli 0 0 sg rule add sg-1 --ip-proto -1 --sg-members sg-2
cli 0 0 sg rule add sg-1 --ip-proto all --sg-members sg-2
cli 0 1 sg rule add --ip-proto 42 --sg-members sg-2
cli 0 1 sg rule add sg-1 --sg-members sg-2
cli 0 1 sg rule add sg-1 --ip-proto 42
cli 0 1 sg rule add sg-1
cli 0 1 sg rule add sg-1 --dir in --ip-proto tcp --port-start 1000 --cidr 10.0.0.0/24
cli 0 1 sg rule add sg-1 --dir in --ip-proto tcp --port-end 1000 --cidr 10.0.0.0/24
cli 0 1 sg rule add sg-1 --ip-proto 256 --sg-members sg-2
cli 0 1 sg rule add sg-1 --ip-proto not_a_proto --sg-members sg-2
cli 0 1 sg rule add sg-1 --ip-proto tcp --sg-members sg-2
cli 0 1 sg rule add sg-1 --ip-proto udp --sg-members sg-2
cli 0 1 sg rule add sg-1 --ip-proto tcp --port -1 --cidr 10.0.0.0/24
cli 0 1 sg rule add sg-1 --ip-proto tcp --port 65536 --cidr 10.0.0.0/24
cli 0 1 sg rule add sg-1 --ip-proto tcp --port not_a_port --cidr 10.0.0.0/24
cli 0 1 sg rule add sg-1 --dir in --ip-proto tcp --port-start not_a_port --port-end 2000 --cidr 10.0.0.0/24
cli 0 1 sg rule add sg-1 --dir in --ip-proto tcp --port-start 1000 --port-end not_a_port --cidr 10.0.0.0/24
cli 0 1 sg rule add sg-1 --dir in --ip-proto tcp --port-start 2000 --port-end 1000 --cidr 10.0.0.0/24
cli 0 1 sg rule add sg-1 --ip-proto 42 --sg-members sg-2 --dir out # not supported yet
cli 0 1 sg rule add sg-1 --ip-proto 42 --sg-members sg-2 --dir not_a_dir

cli 0 1 sg rule list
cli 0 0 sg rule list help
cli 0 0 sg rule list sg-1
cli 0 0 sg rule list sg-2
cli 0 0 sg rule list sg-4
cli 0 1 sg rule list sg-not-here

cli 0 1 sg rule del
cli 0 0 sg rule del help
cli 0 0 sg rule del sg-1 f44fac34f8667927
cli 0 0 sg rule del sg-1 cfdb94f9418c88f1
cli 0 1 sg rule del sg-1 f44fac34f8667927
cli 0 1 sg rule del not-a-sg
cli 0 1 sg rule del sg_1 not-a-hash

cli 0 1 sg member
cli 0 0 sg member help

cli 0 1 sg member add
cli 0 0 sg member add help
cli 0 0 sg member add sg-1 10.0.0.1
cli 0 0 sg member add sg-1 10.0.0.2
cli 0 0 sg member add sg-1 10.0.0.1
cli 0 0 sg member add sg-1 2001:0db8:0000:85a3:0000:0000:ac1f:8001
cli 0 0 sg member add sg-1 2001:db8:0:85a3::ac1f:8001
cli 0 0 sg member add sg-1 2001:db8:0:85a3::ac1f:8002
cli 0 0 sg member add sg-6 10.0.0.1
cli 0 1 sg member add sg-1
cli 0 1 sg member add sg-1 not_an_ip

cli 0 1 sg member list
cli 0 0 sg member list help
cli 0 0 sg member list sg-1
cli 0 0 sg member list sg-2
cli 0 0 sg member list sg-6
cli 0 1 sg member list not-a-sg

cli 0 1 sg member del
cli 0 0 sg member del help
cli 0 0 sg member del sg-1 10.0.0.1
cli 0 0 sg member del sg-1 10.0.0.1
cli 0 0 sg member del sg-1 2001:0db8:0000:85a3:0000:0000:ac1f:8002
cli 0 0 sg member del not-a-sg 10.0.0.1
cli 0 0 sg member del sg-1 172.0.0.1
cli 0 1 sg member del sg-1 
cli 0 1 sg member del sg-1 not_an_ip

cli 0 1 sg del
cli 0 0 sg del help
cli 0 0 sg del sg-1
cli 0 0 sg del sg-2 sg-3
cli 0 0 sg del sg-4 not-a-sg
cli 0 0 sg del not-a-sg

cli 0 0 status help
cli 0 0 status
cli 0 0 status
cli 0 0 status

cli 0 0 shutdown help
cli 0 0 shutdown

network_disconnect 0 1
return_result

