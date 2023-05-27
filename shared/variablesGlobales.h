#ifndef KERNEL_VARIABLES_GLOBALES_H_
#define KERNEL_VARIABLES_GLOBALES_H_
#include<commons/log.h>
#include<commons/collections/list.h>

#include"structs.h"

int conexionCPU;
int conexionMemoria;
int conexionFileSystem;
int servidorKernel;

t_list* lista_estados[5]; // TODO: Usar constante CANTIDAD_ESTADOS

extern t_log* kernelLogger;
extern t_kernel_config* kernel_config;
int contadorProcesoId = 0;

#endif
