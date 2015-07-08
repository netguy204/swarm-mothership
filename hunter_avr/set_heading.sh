#!/bin/bash

curl -X POST --data "{\"type\": \"SET_HEADING\", \"heading\": $1, \"pid\":100}" http://raspberrypi:8080/commands --header "Content-Type:application/json"
