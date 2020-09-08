#!/bin/bash

for N in {1..99}
do
    ruby client.rb $N &
done
wait

