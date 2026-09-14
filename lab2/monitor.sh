#!/bin/bash

LOGFILE="performance.log"
INTERVAL=5

echo "Performance Monitoring Started at $(date)" > "$LOGFILE"

while true
do
{
echo "=========================================="
echo "Timestamp : $(date)"
echo

echo "----- CPU Statistics -----"
cat /proc/stat | head -5

echo
echo "----- Memory Information -----"
grep -E "MemTotal|MemFree|MemAvailable|Buffers|Cached|SwapTotal|SwapFree" /proc/meminfo

echo
echo "----- Load Average -----"
cat /proc/loadavg

echo
echo "----- Uptime -----"
cat /proc/uptime

echo
echo "----- Disk Statistics -----"
cat /proc/diskstats

echo
echo "----- Network Statistics -----"
cat /proc/net/dev

echo "=========================================="
echo

} >> "$LOGFILE"

sleep $INTERVAL

done