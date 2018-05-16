#!/bin/bash
count=0
udp_port=20000
while [ $count -lt $1 ]; do
    if [ $udp_port == 20000 ]; then
        ./server_start $udp_port >> ./data/$count.txt &
        let count+=1
    fi
    father=$udp_port
    let udp_port+=1
    ./server_start $udp_port localhost $father  >> ./data/$count.txt &
    let count+=1
    sleep 1
done
