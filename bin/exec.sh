#!/bin/bash
TPHOME=/home/utnso/eclipse-workspace/tp-2023-1c-KernelPanic
PRUEBAS=$TPHOME/tuki-pruebas
FILE=$1
TIPO_PLANIFICADOR=$2
DIR=FILE

# TODO Automatizar la compilacion de los modulos

function run_module {
	if ["$FILE" = "consola"]; then 
    	../$DIR/$FILE $CONFIG_PATH $INSTRUCCIONES
    else 
    	../$DIR/$FILE $CONFIG_PATH
    fi;
}

if ["$TIPO_PLANIFICADOR" = "FIFO"]; then
	CONFIG_PATH=$PRUEBAS/planificador_fifo/$FILE.config
	INSTRUCCIONES=$PRUEBAS/planificador_fifo/instrucciones.txt
	run_module;
fi;
if ["$TIPO_PLANIFICADOR" = "HRRN"]; then
	CONFIG_PATH=$PRUEBAS/planificador_hrrn/$FILE.config
	INSTRUCCIONES=$PRUEBAS/planificador_hrrn/instrucciones.txt
	run_module;
fi;
