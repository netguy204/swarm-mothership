#!/bin/bash

HUNTER=${1:-10} # Hunter X
HEADING=${2:-2} # double header

curl -X POST --data "{\"type\": \"SET_HEADING\", \"duration\": 3000, \"heading\": $HEADING, \"pid\":$HUNTER}" http://${HOST:-192.168.168.100}:8080/commands --header "Content-Type:application/json"
