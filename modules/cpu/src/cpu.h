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
#include <shared/shared.h>

typedef struct {
	int RETARDO_INSTRUCCION;
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	int PUERTO_ESCUCHA;
	int TAM_MAX_SEGMENTO;
}cpu_config_t;

cpu_config_t* configCpu;

extern registros_cpu* registrosCpu;

void atender_kernel(int);

void cargar_config(t_config*);
int ejecutar_instruccion(char** , PCB*);
void ejecutar_proceso(PCB* pcb, int);
void cargar_registros(PCB* pcb);
char* fetch_instruccion(PCB* pcb);
void guardar_contexto_de_ejecucion(PCB*);
void handshake_memoria(int);
void inicializar_registros();
void set_registro(char*, char*);
void set_registros(PCB* pcb);
void instruccion_set(char* registro,char* valor);

void enviar_pcb_desalojado_a_kernel(PCB*, int, codigo_operacion);

pthread_mutex_t m_recibir_pcb;

// LOGS ////////////////////////
#define INSTRUCCION_EJECUTADA        "PID: <%d> - Ejecutando: <INSTRUCCION> - <PARAMETROS>"
#define ACCESO_MEMORIA               "PID: <%d> - Acción: <LEER / ESCRIBIR> - Segmento: <NUMERO SEGMENTO> - Dirección Física: <DIRECCION FISICA> - Valor: <VALOR LEIDO / ESCRITO>"
#define ERROR_SEGMENTATION_FAULT     "PID: <%d> - Error SEG_FAULT- Segmento: <NUMERO SEGMENTO> - Offset: <OFFSET> - Tamaño: <TAMAÑO>"
////////////////////////////////

#define DEFAULT_LOG_PATH      "logs/cpu.log"
#define DEFAULT_CONFIG_PATH   "tuki-pruebas/prueba-memoria/cpu.config"

uint32_t obtener_direccion_fisica(PCB *pcb,uint32_t dirLogica, uint32_t *numeroSegmento, uint32_t *offset, uint32_t *tamanioSegmento);
void* convertir_dir_logica_a_fisica(PCB *pcb, char* dirLogica);
void* obtener_base_segmento(PCB *pcb, uint32_t numeroSegmento,  uint32_t *tamanio);
void* obtener_puntero_direccion_fisica(PCB *pcb,uint32_t dirLogica, uint32_t *numeroSegmento, uint32_t *offset, uint32_t *tamanioSegmento);

#endif
