#ifndef MEMORIA_H_
#define MEMORIA_H_

// Include externas
#include <pthread.h>

// Include internas
#include "constantes.h"
#include "../../shared/structs.h"
#include "../../shared/constantes.h"
#include "../../shared/constructor.h"
#include "../../shared/funciones.h"
#include "../../shared/funcionesCliente.h"
#include "../../shared/funcionesServidor.h"
#include "../../shared/variablesGlobales.h"



typedef struct {
	int PUERTO_ESCUCHA;
    int TAM_MEMORIA;
    int TAM_SEGMENTO_0;
    int CANT_SEGMENTOS;
    int RETARDO_MEMORIA;
    int RETARDO_COMPACTACION;
    char* ALGORITMO_ASIGNACION;
} t_memoria_config;

void cargar_config_memoria(t_config*);
void ejecutar_file_system_pedido(void *);
void ejecutar_cpu_pedido(void *);
void ejecutar_kernel_pedido(void *);
void inicializar_segmento_generico();
void iterator(char*);
void ejecutar_instrucciones_memoria(int, char*);

#endif
