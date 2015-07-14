#!/bin/bash

HUNTER=$1
HEADING=$2
SPEED=$3
DURATION=$4

curl -X POST --data "{\"type\": \"DRIVE\", \"duration\": $DURATION, \"heading\": $HEADING, \"speed\": $SPEED, \"pid\":$HUNTER}" http://192.168.168.100:8080/commands --header "Content-Type:application/json"
