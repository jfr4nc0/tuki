#!/bin/bash
FILE=fileSystem
make $FILE
if test -f "./$FILE"; then
    ./$FILE ../tuki-pruebas/prueba-base/CPU.config
fi