#!/bin/bash

set -e

echo current dir $(pwd)

echo packetgraph source dir: $1
PG_SOURCE_DIR=$1

echo packetgraph install dir: $2
PG_INSTALL_DIR=$2

cd $PG_SOURCE_DIR

git submodule sync
git submodule update --init --recursive

echo dpdk dir: $RTE_SDK
$PG_SOURCE_DIR/configure --prefix "$PG_INSTALL_DIR" CC=clang --with-examples EXTRA_CFLAGS="-march=ivybridge -DPG_BRICK_NO_ATOMIC_COUNT -O2 -g"

