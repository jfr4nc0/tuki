#ifndef CPU_H_
#define CPU_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <commons/log.h>
    #include <commons/string.h>
    #include <commons/config.h>
	#include <pthread.h>

	// Internas
//	#include "../../kernel/include/kernel.h"

	#include "../../shared/funciones.h"
	#include "../../shared/funcionesCliente.h"
	#include "../../shared/funcionesServidor.h"
	#include "../../shared/constructor.h"
	#include "../../shared/constantes.h"
	#include "../../shared/structs.h"
    #include "../../shared/variablesGlobales.h"
    #include "constantes.h"

typedef struct {
	char* RETARDO_INSTRUCCION;
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* PUERTO_ESCUCHA;
	int TAM_MAX_SEGMENTO;
}cpu_config_t;

cpu_config_t* configCpu;

extern registros_cpu* registrosCpu;


void cargar_config(t_config*);
char** decode_instruccion(char*);
void ejecutar_instruccion();
void ejecutar_proceso(PCB* pcb);
char* fetch_instruccion(PCB* pcb);
void guardar_contexto_de_ejecucion(PCB*);
void handshake_memoria(int);
void inicializar_registros();
void* procesar_instruccion(int);
void set_registro(char*, char*);
void set_registros(PCB* pcb);
PCB* recibir_pcb(int);

#endif
