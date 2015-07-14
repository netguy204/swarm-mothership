#!/bin/bash

HUNTER=$1
HEADING=$2

curl -X POST --data "{\"type\": \"SET_HEADING\", \"duration\": 3000, \"heading\": $HEADING, \"pid\":$HUNTER}" http://192.168.168.100:8080/commands --header "Content-Type:application/json"
