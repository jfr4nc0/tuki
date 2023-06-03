#ifndef KERNEL_H_
#define KERNEL_H_

#include <shared/shared.h>
#include "planificador.h"

t_list* procesar_instrucciones(int, t_list*, t_log*, t_config*);
void cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
void* recibir_de_consola(void*);
void iterator(char* value);


PCB* new_pcb(t_list* , int);

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

#define PATH_LOG_KERNEL             "./logs/kernel.log"
#define PATH_CONFIG_KERNEL          "../../tuki-pruebas/prueba-base/kernel.config"


#endif
