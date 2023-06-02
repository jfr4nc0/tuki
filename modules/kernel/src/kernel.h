#ifndef KERNEL_H_
#define KERNEL_H_

// Librerias externas
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/log.h>
#include <commons/config.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <pthread.h>

#include <shared/shared.h>

int conexionCPU;
int conexionMemoria;
int conexionFileSystem;
int servidorKernel;

t_list* lista_estados[5]; // TODO: Usar constante CANTIDAD_ESTADOS
sem_t sem_lista_estados[5];

pthread_mutex_t m_listas_mutex[5];

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;
int contadorProcesoId = 0;

t_list* procesar_instrucciones(int, t_list*, t_log*, t_config*);
void cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
void* recibir_de_consola(void*);
void iterator(char* value);

PCB* inicializar_pcb(int, t_list*);

PCB* new_pcb(int, t_list*);
///////////////////////////


/* Scheduler */

const char* nombres_estados[] = {
        NEW,
        READY,
        BLOCKED,
        EXECUTING,
        EXIT
};

const char* obtener_nombre_estado(pcb_estado estado){
	if (estado >= ENUM_NEW) {
		return nombres_estados[estado];
	}
	return "EL ESTADO NO ESTÁ REGISTRADO"; //TODO: Mejorar este mensaje
}

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

t_list* lista_estados[CANTIDAD_ESTADOS];
sem_t sem_lista_estados[CANTIDAD_ESTADOS];


/////// LOGS OBLIGATORIOS///////////
#define ABRIR_ARCHIVO               "PID: <PID> - Abrir Archivo: <NOMBRE ARCHIVO>"
#define ACTUALIZAR_PUNTERO_ARCHIVO     "PID: <PID> - Actualizar puntero Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO>" // Nota: El valor del puntero debe ser luego de ejecutar F_SEEK.
#define CAMBIO_DE_ESTADO            "PID: <PID> - Estado Anterior: <ESTADO_ANTERIOR> - Estado Actual: <ESTADO_ACTUAL>"
#define CERRAR_ARCHIVO              "PID: <PID> - Cerrar Archivo: <NOMBRE ARCHIVO>"
#define CREACION_DE_PROCESO         "Se crea el proceso <PID> en NEW"
#define CREAR_SEGMENTO                 "PID: <PID> - Crear Segmento - Id: < id SEGMENTO> - Tamaño: <TAMAÑO>"
#define ELIMINAR_SEGMENTO           "PID: <PID> - Eliminar Segmento - Id Segmento: < id SEGMENTO>"
#define ESCRIBIR_ARCHIVO            "PID: <PID> -  Escribir Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>"
#define FIN_COMPACTACIÓN            "Se finalizó el proceso de compactación"
#define FIN_DE_PROCESO              "Finaliza el proceso <PID> - Motivo: <SUCCESS / SEG_FAULT / OUT_OF_MEMORY>"
#define I_O                         "PID: <PID> - Ejecuta IO: <TIEMPO>"
#define INGRESO_A_READY             "Cola Ready <ALGORITMO>: [<LISTA DE PIDS>]"
#define INICIO_COMPACTACIÓN         "Compactación: <Se solicitó compactación / Esperando Fin de Operaciones de FS>"
#define LEER_ARCHIVO                "PID: <PID> - Leer Archivo: <NOMBRE ARCHIVO> - Puntero <PUNTERO> - Dirección Memoria <DIRECCIÓN MEMORIA> - Tamaño <TAMAÑO>"
#define MOTIVO_DE_BLOQUEO           "PID: <PID> - Bloqueado por: <IO / NOMBRE_RECURSO / NOMBRE_ARCHIVO>"
#define SIGNAL                      "PID: <PID> - Signal: <NOMBRE RECURSO> - Instancias: <INSTANCIAS RECURSO>" // Nota: El valor de las instancias es después de ejecutar el Signal
#define TRUNCAR_ARCHIVO             "PID: <PID> - Archivo: <NOMBRE ARCHIVO> - Tamaño: <TAMAÑO>"
#define WAIT                        "PID: <PID> - Wait: <NOMBRE RECURSO> - Instancias: <INSTANCIAS RECURSO>" // Nota: El valor de las instancias es después de ejecutar el Wait
////////////////////////////////////

#define PATH_LOG_KERNEL             "../logs/kernel.log"
#define PATH_CONFIG_KERNEL          "kernel.config"


#endif
