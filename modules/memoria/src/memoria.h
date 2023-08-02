#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <commons/collections/list.h>

#include <shared/shared.h>
#include "compartido.h"

t_log* loggerMemoria;

pthread_mutex_t mutex_memoria_ocupada;

typedef struct {
	int PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_SEGMENTO_0;
    int CANT_SEGMENTOS;
    int RETARDO_MEMORIA;
    int RETARDO_COMPACTACION;
    char* ALGORITMO_ASIGNACION;
} t_memoria_config;

typedef struct {
	int idProceso;
    size_t size;
} t_parametros_write_read;

#define DEFAULT_LOG_PATH            "logs/memoria.log"
#define DEFAULT_CONFIG_PATH         "tuki-pruebas/prueba-base/memoria.config"

// LOGS ////////////////////////////////////
#define CREACION_DE_PROCESO         "Creación de Proceso PID: %d"
#define ELIMINACION_DE_PROCESO      "Eliminación de Proceso PID: %d"
#define RESULTADO_COMPACTACION      "Por cada segmento de cada proceso se deberá imprimir una línea con el siguiente formato:\n PID: %d - Segmento: %d - Base: %d - Tamaño %d"

#define I__RECIBO_INSTRUCCION        "Me llegaron los siguientes valores para la operacion numero: %d desde %s"

///////////////////////////////////////////

void cargar_config_memoria(t_config*);
void ejecutar_file_system_pedido(void *);
void ejecutar_cpu_pedido(void *);
void ejecutar_kernel_pedido(void *);
void ejecutar_instrucciones(int, char*);
void atender_conexiones(int);
void administrar_cliente(void*);
void administrar_instrucciones(int cliente, codigo_operacion codigoOperacion);
void testing_funciones();
void incializar_estructuras();
t_parametros_write_read* obtenerParametrosWriteRead(t_list* listaRecibida);
#endif
