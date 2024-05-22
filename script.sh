#!/bin/bash

for k in {0..1}; do # Цикл от 0 до 9
    i=$((RANDOM % 10 + 1))
    # g++ client.cpp result.cpp record.cpp common.cpp query.cpp -o client 
    ./client $i.txt
    # echo $i
done
