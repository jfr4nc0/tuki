#!/bin/bash
FILE=kernel
make $FILE
if test -f "./$FILE"; then
    ./$FILE ../tuki-pruebas/prueba-base/Kernel.config
fi