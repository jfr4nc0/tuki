#!/bin/bash

# Menu interactivo para testear el sistema

server_name=$(hostname)

function checkpoint_1() {
	# TODO Armar el deploy
	echo "Implementar"
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

MENU:
$(ColorGreen '1)') Planificacion FIFO (CheckPoint 1)
$(ColorGreen '0)') Exit

$(ColorBlue 'Seleccione el tipo de ejecucion:') "
        read a
        case $a in
	        1) checkpoint_1 ; menu ;;
		0) echo "Exit..."; exit 0 ;;
		*) echo -e $red"Opcion erronea."$clear; WrongCommand;;
        esac
}

# Call the menu function
menu