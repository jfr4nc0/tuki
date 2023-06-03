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

extern t_log* kernelLogger;
extern t_kernel_config* kernelConfig;

t_list* procesar_instrucciones(int, t_list*, t_log*, t_config*);
void cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
void* recibir_de_consola(void*);
void iterator(char* value);
char** leer_arreglo_string(char* , int* );
char* pids_on_ready();

PCB* inicializar_pcb(int, t_list*);

PCB* new_pcb(t_list* , int , int );
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
//PCB* cambio_de_estado(int, pcb_estado estadoAnterior, pcb_estado estadoNuevo);
void cambiar_a(PCB* , pcb_estado , t_list* , sem_t );
void agregar_a_lista_con_sem(PCB *, t_list *, sem_t);
void liberar_listas_estados();
void loggear_cola_ready(char*);
void cambiar_a_ready();
void agregar_pcb_a_paquete(t_paquete* , PCB* );
PCB* remover_de_lista(int, t_list*, sem_t);

void agregar_elemento_a_paquete(t_paquete* , void* );
void agregar_cadena_a_paquete(t_paquete* , char* );
void agregar_long_a_paquete(t_paquete* , void* );
void agregar_longlong_a_paquete(t_paquete* , void* );
void agregar_lista_a_paquete(t_paquete* , t_list* );
void agregar_int_a_paquete(t_paquete* , int );
void agregar_arreglo_a_paquete(t_paquete* , char** );
void agregar_valor_a_paquete(t_paquete* , void* , int );
void agregar_registros_a_paquete(t_paquete* , registros_cpu* );
void envio_pcb(int , PCB* , codigo_operacion );

// TODO: Cambiar por obtener_nombre_estado
const char* estadoToString(pcb_estado estado) {
    switch (estado) {
        case ENUM_NEW:
            return "NEW";
        case ENUM_READY:
            return "READY";
        case ENUM_BLOCKED:
            return "BLOCKED";
        case ENUM_EXECUTING:
            return "EXECUTING";
        case ENUM_EXIT:
            return "EXIT";
        default:
            return "NO SE CONOCE EL ESTADO";
    }
}

////////////////////

#endif
