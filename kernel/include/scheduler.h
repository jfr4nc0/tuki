/*
 * scheduler.h
 *
 *  Created on: Apr 29, 2023
 *      Author: utnso
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "kernel.h"

/*
#include <stdio.h> // Para las funciones de entrada/salida estándar
#include <stdlib.h> // Para funciones de uso general, como malloc y free
#include <pthread.h> // Para el uso de hilos (threads)
#include <semaphore.h> // Para el uso de semáforos
#include <string.h> // Para las funciones de manipulación de cadenas de caracteres
#include <unistd.h> // Para funciones relacionadas con el sistema operativo, como sleep
// #include "../include/t_recurso.h" // Archivo de encabezado personalizado para la definición de recursos
// #include "../include/dictionary.h" // Archivo de encabezado personalizado para el uso de diccionarios
#include<commons/collections/list.h>
#include<commons/log.h>
#include <commons/config.h>

#include "pcb.h"
*/

pthread_t planificador_corto_plazo;
pthread_t thread_memoria;
pthread_t thread_cpu;

pthread_mutex_t m_lista_NEW;
pthread_mutex_t m_lista_READY;
pthread_mutex_t m_lista_READY_FIFO;
pthread_mutex_t m_lista_BLOCKED;
pthread_mutex_t m_lista_EXECUTING;
pthread_mutex_t m_lista_EXIT;
pthread_mutex_t m_lista_IO;

typedef struct{
    char* nombre;
    int instancias;
    // t_list* lista_procesos;
    sem_t sem_recurso;
} t_recurso;

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
void cambio_de_estado(PCB*, pcb_estado, t_list*, pthread_mutex_t);
void cambiar_estado_pcb_a(PCB*, pcb_estado);
void agregar_a_lista(PCB*, t_list*, pthread_mutex_t);

#endif /* SCHEDULER_H_ */
