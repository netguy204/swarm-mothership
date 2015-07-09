#!/bin/bash

HUNTER=$1
HEADING=$2

curl -X POST --data "{\"type\": \"SET_HEADING\", \"heading\": $HEADING, \"pid\":$HUNTER}" http://192.168.168.100:8080/commands --header "Content-Type:application/json"
