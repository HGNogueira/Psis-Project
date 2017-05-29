#!/bin/bash

n_servers=$1

if [ "$#" -eq 1 ]
then
    eval rm server{1..$n_servers} -r
else
    echo "Número de parâmetros errados"
fi
