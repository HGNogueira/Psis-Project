#!/bin/bash

n_servers=$1

if [ "$#" -eq 1 ]
then
    for i in $(seq 1 $n_servers)
    do
        if [ $i -ne $n_servers ]
        then
            (cd ./server"$i" && exec ./server &)
        else
            (cd ./server"$i" && exec ./server)
        fi
    done
else
    echo "Número de parâmetros errados"
fi
