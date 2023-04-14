#ifndef FUNCIONES_GLOBALES_H_
#define FUNCIONES_GLOBALES_H_

#include <commons/config.h>
#include "constantes.h"

char* extraerDeConfig(t_config*, char*, char*, t_log* logger);
char* concatenarStrings(char*, char*);
t_log* iniciar_logger(char*);
t_config* iniciar_config(char*, t_log*);
void leer_consola(t_log*);
void terminar_programa(int, t_log*, t_config*);
void liberar_conexion(int);

#endif
