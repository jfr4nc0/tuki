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

extern t_log* logger;

t_log* logger;
t_log* logger_obligatorio;

int conexionCPU;
int conexionMemoria;
int conexionFileSystem;

pthread_t planificador_corto_plazo;
pthread_t thread_memory;
pthread_t thread_cpu;

pthread_mutex_t m_lista_NEW;
pthread_mutex_t m_lista_READY;
pthread_mutex_t m_lista_READY_FIFO;
pthread_mutex_t m_lista_BLOCKED;
pthread_mutex_t m_lista_EXECUTING;
pthread_mutex_t m_lista_EXIT;
pthread_mutex_t m_lista_IO;

typedef struct
{
    char* IP_KERNEL;
	char* PUERTO_KERNEL;
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

t_list* lista_NEW;
t_list* lista_READY;
t_list* lista_BLOCKED;
t_list* lista_EXECUTING;
t_list* lista_EXIT;
t_list* lista_IO;

typedef struct{
    char* nombre;
    int instancias;
    //t_list* lista_procesos;
    sem_t sem_recurso;
} t_recurso;

t_dictionary* diccionario_recursos;

void inicializar_semaforos();

sem_t sem_grado_multiprogamacion;
sem_t sem_proceso_en_ready;
sem_t sem_cpu_disponible;
sem_t sem_creacion_pcb;
sem_t sem_proceso_a_ready;

void inicializar_planificador();

void proximo_a_ejecutar();
void cambio_de_estado();
// void manage_memory();
// void manage_cpu();




#endif
