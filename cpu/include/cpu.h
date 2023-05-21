#ifndef CPU_H_
#define CPU_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <commons/log.h>
    #include <commons/string.h>
    #include <commons/config.h>
    #include <readline/readline.h>

    #include "constantes.h"
	//#include "../../kernel/include/pcb.h"

typedef struct {
	char* RETARDO_INSTRUCCION;
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* PUERTO_ESCUCHA;
	int TAM_MAX_SEGMENTO;
}cpu_config_t;

cpu_config_t* cpu_config;

extern cpu_registers* registrosCPU;

void handshakeConMemoria(int);
void inicializar_registros(cpu_registers*);
void ejecutar_instruccion();


#endif
