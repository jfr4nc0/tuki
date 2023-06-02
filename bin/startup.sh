#!/bin/bash

# Menu interactivo para testear el sistema

server_name=$(hostname)
TPHOME="/home/utnso/eclipse-workspace/tp-2023-1c-KernelPanic"
BIN_PATH=$TPHOME/bin

function planificador_FIFO() {
	bash $BIN_PATH/exec.sh consola FIFO
	bash $BIN_PATH/exec.sh kernel FIFO
	bash $BIN_PATH/exec.sh cpu FIFO
	bash $BIN_PATH/exec.sh memoria FIFO
	bash $BIN_PATH/exec.sh fileSystem FIFO
}

function planificador_HRRN() {
	bash $BIN_PATH/exec.sh consola HRRN
	bash $BIN_PATH/exec.sh kernel HRRN
	bash $BIN_PATH/exec.sh cpu HRRN
	bash $BIN_PATH/exec.sh memoria HRRN
	bash $BIN_PATH/exec.sh fileSystem HRRN
}

function deployar_modulos() {
	bash $BIN_PATH/deploy.sh
}

##
# Color  Variables
##
green='\e[32m'
blue='\e[34m'
clear='\e[0m'

##
# Color Functions
##

ColorGreen(){
	echo -ne $green$1$clear
}
ColorBlue(){
	echo -ne $blue$1$clear
}

menu(){
echo -ne "
:::::::::::::::::::::::::::::::::::::::::::::::::::::

88888888888   888     888     888    d8P      8888888
    888       888     888     888   d8P         888  
    888       888     888     888  d8P          888  
    888       888     888     888d88K           888  
    888       888     888     8888888b          888  
    888       888     888     888  Y88b         888  
    888   d8b Y88b. .d88P d8b 888   Y88b  d8b   888  
    888   Y8P  'Y88888P'  Y8P 888    Y88b Y8P 8888888

:::::::: The Ultimate Kernel Implementation :::::::::

By: $(ColorGreen 'KernelPanic')

MENU:
$(ColorGreen '1)') Planificacion FIFO
$(ColorGreen '2)') Planificacion HRRN
$(ColorGreen '3)') Deployar modulos
$(ColorGreen '0)') Exit

$(ColorBlue 'Seleccione el tipo de ejecucion:') "
        read a
        case $a in
	        1) planificacion_FIFO ; menu ;;
	        2) planificacion_HRRN ; menu ;;
	        3) deployar_modulos ; menu ;;
		0) echo "Exit..."; exit 0 ;;
		*) echo -e $red"Opcion erronea."$clear; WrongCommand;;
        esac
}

# Call the menu function
menu