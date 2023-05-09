#ifndef KERNEL_H_
#define KERNEL_H_

// Externas
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include <commons/log.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<assert.h>

#include <pthread.h>
#include <semaphore.h>

// Internas
#include "utils.h"
#include "funciones.h"
#include "constantes.h"
#include "../../shared/constantes.h"
#include "../../shared/structs.h"
#include "pcb.h"
#include "scheduler.h"

extern t_log* logger;

t_log* logger;
t_log* logger_obligatorio;

int conexionCPU;
int conexionMemoria;
int conexionFileSystem;

typedef struct
{
    //char* IP_KERNEL;
	//char* PUERTO_KERNEL;
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
    char* IP_FILE_SYSTEM;
    char* PUERTO_FILE_SYSTEM;
    char* IP_CPU;
    char* PUERTO_CPU;
    char* PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    char* ESTIMACION_INICIAL;
    double HRRN_ALFA;
    int GRADO_MAX_MULTIPROGRAMACION;
    char** RECURSOS;
    char** INSTANCIAS_RECURSOS;
} t_kernel_config;

t_kernel_config* kernel_config;

void inicializar_escucha_conexiones_consolas();

#endif
