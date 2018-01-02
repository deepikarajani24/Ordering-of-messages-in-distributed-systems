#!/bin/bash

count=$1;
port=$2;
> InformationAboutProcess.txt
for i in $(seq 1 $count);
do {
echo "port is $port"
echo $port >> InformationAboutProcess.txt
port=$((port+1))
} done

port=$2;
for i in $(seq 1 $count);
do {
echo $port
gnome-terminal -e "bash -c \"./causualMessageOrdering $port 127.0.0.1; exec bash\""

port=$((port+1))
} done
exit 0
