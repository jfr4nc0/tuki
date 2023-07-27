#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

// Externas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>

// Internas
#include "administrarInstrucciones.h"
#include "inicializarEstructuras.h"
#include <shared/shared.h>

t_log* loggerFileSystem;

t_list* listaFCB;

// Functions
void atender_kernel(int);
void ejecutar_instrucciones_kernel(void*);
void iterator(char* value);
char* obtener_mensaje_de_socket(int cliente);
t_archivo_abierto* obtener_archivo_completo_de_socket(int cliente);
void devolver_instruccion_generico(bool funciono, int cliente);

#endif
