#ifndef CPU_H_
#define CPU_H_

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <commons/log.h>
    #include <commons/string.h>
    #include <commons/config.h>
    #include <readline/readline.h>

    #include "utils.h"
    #include "constantes.h"
    #include "../../shared/src/funciones.c"
	//#include "../../kernel/include/pcb.h"

typedef struct {
	char* RETARDO_INSTRUCCION;
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* PUERTO_ESCUCHA;
	int TAM_MAX_SEGMENTO;
}cpu_config_t;

cpu_config_t* cpu_config;

typedef struct{

	char* AX;
	char* BX;
	char* CX;
	char* DX;

	char* EAX;
	char* EBX;
	char* ECX;
	char* EDX;

	char* RAX;
	char* RBX;
	char* RCX;
	char* RDX;

}cpu_register_t;
cpu_register_t* registrosCPU;

void handshakeConMemoria(int);
void inicializar_registros(cpu_register_t*);
void ejecutar_instruccion();


#endif
