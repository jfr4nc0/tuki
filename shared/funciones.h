#ifndef FUNCIONES_GLOBALES_H_
#define FUNCIONES_GLOBALES_H_

#include <commons/config.h>
#include "constantes.h"
#include "structs.h"

char* extraer_de_config(t_config*, char*, t_log* logger);
char* extraer_de_modulo_config(t_config*, char*, char*, t_log* logger);
char* concatenar_strings(char*, char*);
t_log* iniciar_logger(char*, int);
t_config* iniciar_config(char*, t_log*);
void terminar_programa(int, t_log*, t_config*);
void liberar_conexion(int);
bool obtener_valores_para_logger(int, bool*, t_log_level*, char**);

#endif
