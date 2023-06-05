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
#include <kernel.h>
#include <shared/shared.h>

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

// LOGS ////////////////////////
#define INSTRUCCION_EJECUTADA        "PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>"
#define ACCESO_MEMORIA               "PID: <PID> - Acción: <LEER / ESCRIBIR> - Segmento: <NUMERO SEGMENTO> - Dirección Física: <DIRECCION FISICA> - Valor: <VALOR LEIDO / ESCRITO>"
#define ERROR_SEGMENTATION_FAULT     "PID: <PID> - Error SEG_FAULT- Segmento: <NUMERO SEGMENTO> - Offset: <OFFSET> - Tamaño: <TAMAÑO>"
////////////////////////////////

#define DEFAULT_LOG_PATH      "./logs/cpu.log"
#define DEFAULT_CONFIG_PATH   "../../tuki-pruebas/prueba-base/cpu.config"

/**************** INSTRUCCIONES ****************/
void instruccion_mov_in(char* registro,char* dir_logica);
void instruccion_mov_out(char* dir_logica,char* registro);
void instruccion_io(char* tiempo);
void instruccion_f_open(char* nombre_archivo);
void instruccion_f_close(char* nombre_archivo);
void instruccion_f_seek(char* nombre_archivo, char* posicion);
void instruccion_f_read(char* nombre_archivo, char* dir_logica, char* cant_bytes);
void instruccion_f_write(char* nombre_archivo, char* dir_logica, char* cant_bytes);
void instruccion_f_truncate(char* nombre_archivo,char* tamanio);
void instruccion_wait(char* recurso);
void instruccion_signal(char* recurso);
void instruccion_create_segment(char* id_segmento, char* tamanio);
void instruccion_delete_segment(char* id_segmento);
void instruccion_yield();
void instruccion_exit();

#endif