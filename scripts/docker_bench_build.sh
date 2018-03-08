#!/bin/bash

cd $RTE_SDK/

docker build -t dpdk-app-testpmd .
docker build -t dpdk-app-testpmd2 .
