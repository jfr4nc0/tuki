#!/bin/bash 

# TODO Armar los MakeFile en todos los modulos para la compilacion correspondiente, teniendo en cuenta todas sus dependencias

# Compilacion y Deploy de los componentes del sistema
# 1) Llamar a los MakeFiles de los componentes para su compilacion
# 2) Linkear los objetos creados a la carpeta ./bin/objects

TPHOME="/home/utnso/eclipse-workspace/tp-2023-1c-KernelPanic"
#MODULES=$TPHOME"/modules"
SHARED=$TPHOME"/shared"
CONSOLA=$TPHOME"/consola"
KERNEL=$TPHOME"/kernel"
CPU=$TPHOME"/cpu"
MEMORIA=$TPHOME"/memoria"
FILESYSTEM=$TPHOME"/fileSystem"
LOG="/logs"
OBJ="/obj"

echo "Compilando todos los modulos del sistema..."

# Check log and obj
[ ! -d $CONSOLA$LOG ] && mkdir $CONSOLA$LOG 
[ ! -d $CONSOLA$OBJ ] && mkdir $CONSOLA$OBJ 
[ ! -d $CPU$LOG ] && mkdir $CPU$LOG 
[ ! -d $CPU$OBJ ] && mkdir $CPU$OBJ 
[ ! -d $KERNEL$LOG ] && mkdir $KERNEL$LOG 
[ ! -d $KERNEL$OBJ ] && mkdir $KERNEL$OBJ 
[ ! -d $MEMORIA$LOG ] && mkdir $MEMORIA$LOG 
[ ! -d $MEMORIA$OBJ ] && mkdir $MEMORIA$OBJ 
[ ! -d $FILESYSTEM$LOG ] && mkdir $FILESYSTEM$LOG 
[ ! -d $FILESYSTEM$OBJ ] && mkdir $FILESYSTEM$OBJ 
[ ! -d $SHARED$LOG ] && mkdir $SHARED$LOG 
[ ! -d $SHARED$OBJ ] && mkdir $SHARED$OBJ 



# Make all
make -C $CONSOLA
make -C $KERNEL
make -C $CPU
make -C $MEMORIA
make -C $FILESYSTEM
make -C $SHARED

