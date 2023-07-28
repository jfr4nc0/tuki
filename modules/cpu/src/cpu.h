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
#include <cpuInstrucciones.h>
#include <shared/shared.h>

typedef struct {
	int RETARDO_INSTRUCCION;
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	int PUERTO_ESCUCHA;
	int TAM_MAX_SEGMENTO;
}cpu_config_t;

cpu_config_t* configCpu;

enum registro {
    REGISTRO_AX,
    REGISTRO_BX,
    REGISTRO_CX,
    REGISTRO_DX,
    REGISTRO_EAX,
    REGISTRO_EBX,
    REGISTRO_ECX,
    REGISTRO_EDX,
    REGISTRO_RAX,
    REGISTRO_RBX,
    REGISTRO_RCX,
    REGISTRO_RDX,
    REGISTRO_NULL
};

typedef enum registro t_registro;

typedef struct ParametrosLectura {
    int id_proceso;
	uint32_t direccionFisica;
	uint32_t tamanio;
} t_parametros_lectura;

typedef struct ParametrosEScritura {
    int id_proceso;
	uint32_t direccionFisica;
	uint32_t tamanio;
	char* bytes_a_enviar;
} t_parametros_escritura;

extern registros_cpu* registrosCpu;

void atender_kernel(int);
void mostrar_pcb(PCB* );

void cargar_config(t_config*);
int ejecutar_instruccion(char** , PCB*);
void ejecutar_proceso(PCB* pcb, int);
void cargar_registros(PCB* pcb);
char* fetch_instruccion(PCB* pcb);
void guardar_contexto_de_ejecucion(PCB*);
void handshake_memoria(int);
void inicializar_registros();
void procesar_instruccion(void*);
void set_registro(char*, char*);
void set_registros(PCB* pcb);
void instruccion_set(char* registro,char* valor);
char* recibir_valor_a_escribir(int clienteAceptado);
uint32_t obtener_tamanio_segun_registro(char* registro);
char* obtener_valor_registro(char* registro, registros_cpu *registrosCPU);
char *registros_cpu_get_valor_registro(char* registro, int tamanioRegistro);
void log_acceso_a_memoria(uint32_t pid, char* modo, uint32_t idSegmento, uint32_t dirFisica, void* valor, uint32_t tamanio);
void loggear_segmentation_fault(uint32_t pid, uint32_t numSegmento, uint32_t offset, uint32_t tamanio);
uint32_t obtener_direccion_fisica(PCB *pcb,uint32_t dirLogica, uint32_t *numeroSegmento, uint32_t *offset, uint32_t *tamanioSegmento);
uint32_t obtener_base_segmento(PCB *pcb, uint32_t numeroSegmento,  uint32_t *tamanio);

PCB* recibir_pcb(int);
void enviar_pcb_desalojado_a_kernel(PCB*, int, codigo_operacion);
void envio_pcb_a_kernel_con_codigo(int , PCB* , codigo_operacion );
void agregar_pcb_a_paquete(t_paquete* , PCB* );
void* get_registro_cpu(char* registro, registros_cpu* registrosCpu);

// LOGS ////////////////////////
#define INSTRUCCION_EJECUTADA        "PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>"
#define ACCESO_MEMORIA               "PID: <PID> - Acción: <LEER / ESCRIBIR> - Segmento: <NUMERO SEGMENTO> - Dirección Física: <DIRECCION FISICA> - Valor: <VALOR LEIDO / ESCRITO>"
#define ERROR_SEGMENTATION_FAULT     "PID: <PID> - Error SEG_FAULT- Segmento: <NUMERO SEGMENTO> - Offset: <OFFSET> - Tamaño: <TAMAÑO>"
////////////////////////////////

#define DEFAULT_LOG_PATH      "logs/cpu.log"
#define DEFAULT_CONFIG_PATH   "tuki-pruebas/prueba-base/cpu.config"

void devolver_pcb_kernel(PCB*, int, codigo_operacion);

void agregar_registros_a_paquete_para_kernel(t_paquete* , registros_cpu* );
void agregar_registro4bytes_a_paquete(t_paquete* , char[4] );
void agregar_registro8bytes_a_paquete(t_paquete* , char[8] );
void agregar_registro16bytes_a_paquete(t_paquete* , char[16] );



#endif
