#!/bin/bash
g++ -std=c++11 -pthread ring.cpp -o server_start
count=0
udp_port=20000
cat /dev/null > ../data/log.txt
while [ $count -lt $1 ]; do
    if [ $udp_port == 20000 ]; then
        ./server_start $udp_port >> ../data/log.txt &
        let count+=1
    fi
    father=$udp_port
    let udp_port+=1
    ./server_start $udp_port localhost $father  >> ../data/log.txt &
    let count+=1
    sleep 1
done
