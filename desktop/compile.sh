#!/bin/bash

if [ "$1" == "u" ]; then
    cd /source/desktop/schedulers/
    make clean
    make ubuntu
    mv fifo /source/router/schedulers/
    mv sjf /source/router/schedulers/
    mv rr /source/router/schedulers/
    cd /source/desktop/statistics/
    make clean
    make ubuntu
    mv statistics /source/router/
    exit 0
elif [ "$1" == "c" ]; then
    cd /source/desktop/schedulers/
    make clean
    make ubuntu
    mv fifo /source/router/schedulers/
    mv sjf /source/router/schedulers/
    mv rr /source/router/schedulers/
    cd /source/desktop/statistics/
    make clean
    make ubuntu
    mv statistics /source/router/
    exit 0
else
    echo "Error: Invalid argument. Please use 'u' or 'c'."
    exit 1
fi