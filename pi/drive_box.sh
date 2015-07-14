#!/bin/bash

HUNTER=$1
SPEED=$2
DURATION=$3

BASE=`dirname $0`

$BASE/set_heading.sh $HUNTER 0
$BASE/drive.sh $HUNTER 0 $SPEED $DURATION
$BASE/set_heading.sh $HUNTER 90
$BASE/drive.sh $HUNTER 90 $SPEED $DURATION
$BASE/set_heading.sh $HUNTER 180
$BASE/drive.sh $HUNTER 180 $SPEED $DURATION
$BASE/set_heading.sh $HUNTER 270
$BASE/drive.sh $HUNTER 270 $SPEED $DURATION
$BASE/set_heading.sh $HUNTER 0
