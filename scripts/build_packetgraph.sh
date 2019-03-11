#!/bin/bash

set -e

echo current dir $(pwd)

echo packetgraph source dir: $1
PG_SOURCE_DIR=$1

echo packetgraph install dir: $2
PG_INSTALL_DIR=$2

echo dpdk dir: $RTE_SDK

$PG_SOURCE_DIR/autogen.sh $PG_SOURCE_DIR

CFLAGS="-march=ivybridge -DPG_VHOST_FASTER_YET_BROKEN_POLL -DPG_BRICK_NO_ATOMIC_COUNT -O2 -g" $PG_SOURCE_DIR/configure_clang --prefix="$PG_INSTALL_DIR" --with-examples

