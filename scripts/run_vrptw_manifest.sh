#!/usr/bin/env bash

MANIFEST=$1
TIME=$2

mkdir -p results/vrptw

while read line; do

inst=$(echo $line | awk '{print $1}')
veh=$(echo $line | awk '{print $2}')

base=$(basename $inst .txt)

./build-linux/vrptw_vrplib $inst $veh $TIME \
 | tee results/vrptw/${base}_t${TIME}.log

done < $MANIFEST