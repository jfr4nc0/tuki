/*
 * scheduler.h
 *
 *  Created on: Apr 29, 2023
 *      Author: utnso
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_




#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include "../../shared/structs.h"
#include "../../shared/funciones.h"
#include "../../shared/variablesGlobales.h"

pthread_t planificador_corto_plazo;
pthread_t thread_memoria;
pthread_t thread_cpu;

sem_t m_lista_NEW;
sem_t m_lista_READY;
sem_t m_lista_READY_FIFO;
sem_t m_lista_BLOCKED;
sem_t m_lista_EXECUTING;
sem_t m_lista_EXIT;
sem_t m_lista_IO;

typedef struct{
    char* nombre;
    int instancias;
    // t_list* lista_procesos;
    sem_t sem_recurso;
}t_recurso;

t_dictionary* diccionario_recursos;

void inicializar_semaforos();
void crear_cola_recursos(char*, int);
void inicializar_diccionario_recursos();

sem_t sem_grado_multiprogamacion;
sem_t sem_proceso_en_ready;
sem_t sem_cpu_disponible;
sem_t sem_creacion_pcb;
sem_t sem_proceso_a_ready;

void inicializar_planificador();
void inicializar_listas_estados();
void proximo_a_ejecutar();
void cambio_de_estado(PCB*, pcb_estado);
void cambiar_estado_pcb_a(PCB*, pcb_estado);
//void agregar_a_lista(PCB*, t_list*, pthread_mutex_t);

t_list* lista_NEW;
t_list* lista_READY;
t_list* lista_EXECUTING;
t_list* lista_BLOCKED;
t_list* lista_EXIT;


#endif
