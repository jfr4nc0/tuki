#ifndef KERNEL_H_
#define KERNEL_H_

// Externas
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<commons/collections/list.h>
#include<commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

// Internas
#include "funciones.h"
#include "constantes.h"
#include "../../shared/constantes.h"
#include "../../shared/funciones.h"
#include "../../shared/funcionesCliente.h"
#include "../../shared/funcionesServidor.h"
#include "../../shared/structs.h"
#include "../../shared/constructor.h"

int conexionCPU;
int conexionMemoria;
int conexionFileSystem;
int servidorKernel;

t_list* lista_estados[CANTIDAD_ESTADOS];

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

extern t_kernel_config kernel_config;
extern t_log* logger;

t_kernel_config inicializar_config(char*, t_log*);
t_kernel_config cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
PCB* inicializar_pcb(int, t_list*);
void* recibir_de_consola(void*);
void iterator(char* value);

#endif
