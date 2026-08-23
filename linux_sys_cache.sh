#!/bin/bash


for d in /sys/devices/system/cpu/cpu0/cache/index*/*; do

echo $d
cat $d

done