#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <commons/collections/list.h>

#include <shared/shared.h>
#include "compartido.h"

t_log* loggerMemoria;

typedef struct {
	int PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_SEGMENTO_0;
    int CANT_SEGMENTOS;
    int RETARDO_MEMORIA;
    int RETARDO_COMPACTACION;
    char* ALGORITMO_ASIGNACION;
} t_memoria_config;

#define DEFAULT_LOG_PATH            "logs/memoria.log"
#define DEFAULT_CONFIG_PATH         "tuki-pruebas/prueba-base/memoria.config"

// LOGS ////////////////////////////////////
#define CREACION_DE_PROCESO         "Creación de Proceso PID: <PID>"
#define ELIMINACION_DE_PROCESO      "Eliminación de Proceso PID: <PID>"
#define CREACION_DE_SEGMENTO        "PID: %s - Crear Segmento: %d - Base: %d - TAMAÑO: %d"
#define RESULTADO_COMPACTACION      "Por cada segmento de cada proceso se deberá imprimir una línea con el siguiente formato:\n PID: <PID> - Segmento: < id SEGMENTO> - Base: <BASE> - Tamaño <TAMAÑO>"

#define I__RECIBO_INSTRUCCION        "Me llegaron los siguientes valores para la operacion numero: %d desde %s"

///////////////////////////////////////////

void cargar_config_memoria(t_config*);
void ejecutar_file_system_pedido(void *);
void ejecutar_cpu_pedido(void *);
void ejecutar_kernel_pedido(void *);
void iterator(char*);
void ejecutar_instrucciones(int, char*);
void atender_conexiones(int);
void administrar_cliente(int, int);
void administrar_instrucciones(int cliente, codigo_operacion codigoOperacion);

#endif
