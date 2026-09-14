#!/bin/bash

echo "=============================="
echo "      System Information      "
echo "=============================="

echo "-> OS Version:"
uname -a

echo -e "\n-> CPU Information:"
lscpu | grep 'Model name'

echo -e "\n-> Memory Usage:"
free -m

echo -e "\n-> Disk Usage:"
df -h

echo -e "\n-> Network Details:"
ip addr

echo "=============================="
