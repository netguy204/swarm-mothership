#!/bin/bash

HUNTER=${1:-20}     # Wumpus
HEADING=${2:-6}     # Kevin Bacon
SPEED=${3:-88}      # "When this baby hits 88 miles per hour..."
DURATION=${4:-9001} # Over 9000

curl -X POST --data "{\"type\": \"DRIVE\", \"duration\": $DURATION, \"heading\": $HEADING, \"speed\": $SPEED, \"pid\":$HUNTER}" http://${HOST:-192.168.168.100}:8080/commands --header "Content-Type:application/json"
