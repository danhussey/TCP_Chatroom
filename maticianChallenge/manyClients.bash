#!/bin/bash

for N in {1..98}
do
    ruby client.rb $N >/dev/null &
done
wait

