#ifndef UTILS_CONSOLA_H_
	#define UTILS_CONSOLA_H_

	#define DEFAULT_CONFIG_PATH "kernel_conexion.config"
	#define DEFAULT_INSTRUCCIONES_PATH "instrucciones.txt"
	#define DEFAULT_LOG_PATH "logs/consola.log"


	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <commons/log.h>
	#include <commons/string.h>
	#include <commons/config.h>
	#include <readline/readline.h>
	#include<signal.h>
	#include<unistd.h>
	#include<sys/socket.h>
	#include<netdb.h>

	#include "../../shared/funcionesCliente.h"
	#include "../../shared/constantes.h"

//int validarArgumentos(int, char**);

#endif
