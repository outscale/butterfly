#!/bin/bash

BUTTERFLY_BUILD_ROOT=$1
BUTTERFLY_SRC_ROOT=$(cd "$(dirname $0)/../../.." && pwd)
source $BUTTERFLY_SRC_ROOT/tests/functions.sh

network_connect 0 1
key=$BUTTERFLY_BUILD_ROOT/api-key
bad_key=$BUTTERFLY_BUILD_ROOT/bad-api-key
dd if=/dev/urandom bs=32 count=1 | base64 > $key
dd if=/dev/urandom bs=32 count=1 | base64 > $bad_key
server_start_options 0 -k $key -t
server_start 1

cli 0 1 status -k doesnotexits
cli 0 0 nic add --ip 42.0.0.1 --mac 42:42:42:42:42:01 --vni 1337 --id nic_1 -k $key
cli 0 0 nic list -k $key
cli 0 1 nic list
cli 0 0 nic stats nic_1 --key $key
cli 0 1 shutdown
cli 0 1 shutdown -k $bad_key
cli 0 0 shutdown -k $key

cli 1 0 status -k doesnotexits
cli 1 1 nic add --ip 42.0.0.1 --mac 42:42:42:42:42:01 --vni 1337 --id nic_1 -k $key
cli 1 1 nic list -k $key
cli 1 0 nic list
cli 1 1 nic stats nic_1 --key $key
cli 1 1 shutdown -k $key
cli 1 0 shutdown

rm $key

network_disconnect 0 1
return_result

