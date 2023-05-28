#ifndef KERNEL_H_
#define KERNEL_H_

// Librerias externas
#include "includesExternas.h"

// Librerias Internas
#include "constantes.h"
#include "../../shared/funciones.h"
#include "../../shared/funcionesCliente.h"
#include "../../shared/funcionesServidor.h"
#include "../../shared/constructor.h"
#include "../../shared/constantes.h"
#include "../../shared/structs.h"
#include "../../shared/variablesGlobales.h"

t_list* procesar_instrucciones(int, t_list*, t_log*, t_config*);
void cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
void* recibir_de_consola(void*);
void iterator(char* value);

PCB* inicializar_pcb(int, t_list*);

PCB* new_pcb(int, t_list*);
///////////////////////////


/* Scheduler */

pthread_t planificador_corto_plazo;
pthread_t thread_memoria;
pthread_t thread_cpu;

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
PCB* cambio_de_estado(int, pcb_estado estadoAnterior, pcb_estado estadoNuevo);
void agregar_a_lista(PCB*, t_list*, sem_t);
void liberar_listas_estados();

PCB* remover_de_lista(int, t_list*, sem_t);
////////////////////

#endif
