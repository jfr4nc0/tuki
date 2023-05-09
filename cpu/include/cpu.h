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

typedef struct {
	char* RETARDO_INSTRUCCION;
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* PUERTO_ESCUCHA;
	int TAM_MAX_SEGMENTO;
}cpu_config_t;

cpu_config_t* cpu_config;

typedef struct Registros{

	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;

	uint64_t EAX;
	uint64_t EBX;
	uint64_t ECX;
	uint64_t EDX;

	__int128_t RAX;
	__int128_t RBX;
	__int128_t RCX;
	__int128_t RDX;

}registros;

void handshakeConMemoria(int);
void inicializar_registros();
void ejecutar_instruccion();


#endif
