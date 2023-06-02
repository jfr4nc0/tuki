#!/bin/bash
FILE=$1
make $FILE
if test -f "./$FILE"; then
    valgrind --tool=helgrind ./objects/$FILE
fi