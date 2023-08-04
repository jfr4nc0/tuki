#ifndef CONSOLA_GENERAL_H_
#define CONSOLA_GENERAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <signal.h>
#include <unistd.h>

#include <shared/shared.h>

void enviarInstrucciones(char*, int, t_log*);
int armar_conexion_kernel(t_config* config, char* modulo, t_log* logger);
int crear_conexion_con_kernel(char *ip, char* puerto);

#define DEFAULT_INSTRUCCIONES_PATH   "tuki-pruebas/prueba-memoria/MEMORIA_1.txt"
#define DEFAULT_LOG_PATH             "../../../logs/consola.log"

#define DEFAULT_PATH_CONFIG          "tuki-pruebas/prueba-memoria/consola.config"

#endif
