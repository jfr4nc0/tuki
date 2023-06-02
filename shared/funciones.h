#ifndef FUNCIONES_GLOBALES_H_
#define FUNCIONES_GLOBALES_H_

#include <commons/config.h>
#include <commons/log.h>

#include "constantes.h"
#include "structs.h"
#include "configuraciones.h"


char* cantidad_strings_a_mostrar(int);
char* extraer_string_de_config(t_config*, char*, t_log* logger);
char* extraer_de_modulo_config(t_config*, char*, char*, t_log* logger);
char* concatenar_strings(char*, char*);
t_log* iniciar_logger(char*, int);
t_config* iniciar_config(char*, t_log*);
void terminar_programa(int, t_log*, t_config*);
void liberar_conexion(int);
bool obtener_valores_para_logger(int, bool*, t_log_level*, char**);
long leer_long(char* buffer, int* desp);
long long leer_long_long(char* buffer, int* desp);
float leer_float(char* buffer, int* desp);
int leer_int(char* buffer, int* desp);
char* leer_string(char* buffer, int* desp);
t_list* leer_string_array(char* buffer, int* desp);
const char* obtener_nombre_estado(pcb_estado);

#endif
