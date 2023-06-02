#!/bin/bash
FILE=kernel
DIR=./Debug

if test -f "$DIR/$FILE"; then
    cd $DIR
    valgrind --tool=memcheck --leak-check=yes --show-possibly-lost=no --show-reachable=no --num-callers=20 ./$FILE
fi
