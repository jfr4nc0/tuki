#!/bin/bash
FILE=memoria
make $FILE
if test -f "./$FILE"; then
    ./$FILE ../tuki-pruebas/prueba-base/CPU.config
fi
