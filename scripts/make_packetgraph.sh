#!/bin/bash

set -e

PG_SOURCE_DIR=$1

CMD=$2

cd $PG_SOURCE_DIR
make $CMD
