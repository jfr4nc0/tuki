#ifndef KERNEL_VARIABLES_GLOBALES_H_
#define KERNEL_VARIABLES_GLOBALES_H_
#include<commons/log.h>
#include<commons/collections/list.h>
#include <semaphore.h>

#include"structs.h"

int conexionCPU;
int conexionMemoria;
int conexionFileSystem;
int servidorKernel;

t_list* lista_estados[CANTIDAD_ESTADOS];
sem_t m_listas[CANTIDAD_ESTADOS];

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;
int contadorProcesoId = 0;

#endif
