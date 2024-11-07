#!/bin/bash

# Spin up many clients to connect to the chatroom quickly
for N in {1..99}
do
    ruby client.rb $N &
done
wait

