#ifndef KERNEL_H_
#define KERNEL_H_

// Librerias externas
#include "includesExternas.h"

// Librerias Internas
#include "../../shared/funciones.h"
#include "../../shared/funcionesCliente.h"
#include "../../shared/funcionesServidor.h"
#include "constantes.h"
#include "../../shared/constructor.h"
#include "../../shared/constantes.h"
#include "../../shared/structs.h"
#include "variablesGlobales.h"

extern t_log* kernelLogger;
extern t_kernel_config* kernel_config;



t_list* procesar_instrucciones(int, t_list*, t_log*, t_config*);
void cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
void* recibir_de_consola(void*);
void iterator(char* value);

PCB* inicializar_pcb(int, t_list*);


/* PCB */
t_list* pid_list;

int contadorProcesoId = 0;

typedef struct {
	int ID;
	// direccion_base, de que tipo??
	int tamanio;
}t_segmento;

typedef struct {
    int ID; // File descriptor
    int posicion_puntero;
} archivo_abierto_t;

PCB* new_pcb(int, t_list*);
///////////////////////////


/* Scheduler */

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
////////////////////

#endif
