#!/bin/sh

# Butterfly root
BUTTERFLY_BUILD_ROOT=$1
VERBOSE=$2
BUTTERFLY_ROOT=$(cd "$(dirname $0)/.." && pwd)

# Test Butterfly build root
if [ ! -f $BUTTERFLY_BUILD_ROOT/CMakeCache.txt ]; then
    echo "Wrong build directory ?"
    exit 1
fi

# API tests
$BUTTERFLY_ROOT/scripts/tests_api.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "API test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "API test OK"
    tput setaf 7
fi

# Test Protocol ICMP
$BUTTERFLY_ROOT/scripts/tests_icmp.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "ICMP test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "ICMP test OK"
    tput setaf 7
fi

# Test Protocol TCP
$BUTTERFLY_ROOT/scripts/tests_tcp.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "TCP test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "TCP test OK"
    tput setaf 7
fi

# Test Protocol UDP
$BUTTERFLY_ROOT/scripts/tests_udp.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "UDP test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "UDP test OK"
    tput setaf 7
fi

# Test Protocol SCTP
$BUTTERFLY_ROOT/scripts/tests_sctp.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "SCTP test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "SCTP test OK"
    tput setaf 7
fi

# Test Protocol UDP & TCP
$BUTTERFLY_ROOT/scripts/tests_udp_tcp.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "UDP & TCP test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "UDP & TCP test OK"
    tput setaf 7
fi

# Test Firewall
$BUTTERFLY_ROOT/scripts/tests_firewall.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "Firewall test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "Firewall test OK"
    tput setaf 7
fi

# Test Other
$BUTTERFLY_ROOT/scripts/tests_other.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "Other test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "Other test OK"
    tput setaf 7
fi

# Test List
$BUTTERFLY_ROOT/scripts/tests_list.sh $BUTTERFLY_BUILD_ROOT $VERBOSE
if [ $? != 0 ]; then
    tput setaf 1
    echo "List test failed"
    tput setaf 7
    exit 1
else
    tput setaf 2
    echo "List test OK"
    tput setaf 7
fi
