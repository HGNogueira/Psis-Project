#!/bin/bash

n_servers=$1

if [ "$#" -eq 1 ]
then
    for i in $(seq 1 $n_servers)
    do
        mkdir server"$i"
        cp server gw-server.txt ./server"$i"
    done
else
    echo "Número de parâmetros errados"
fi
