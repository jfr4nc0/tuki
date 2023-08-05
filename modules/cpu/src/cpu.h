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
	int direccionFisica;
	int tamanio;
} t_parametros_lectura;

typedef struct ParametrosEScritura {
    int id_proceso;
	uint32_t direccionFisica;
	uint32_t tamanio;
	char* bytes_a_enviar;
} t_parametros_escritura;

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
int obtener_tamanio_registro(char* registro);
int codigo_registro(char* registro);
int obtener_direcc_fisica(PCB* pcb, int dirLogica, int tamanio_registro);
int division_entera(int operando1, int operando2);
void instruccion_set(char* registro,char* valor);

char* recibir_valor_a_escribir(int clienteAceptado);
uint32_t obtener_tamanio_segun_registro(char* registro);
char* obtener_valor_registro(char* registro, registros_cpu *registrosCPU);
char *registros_cpu_get_valor_registro(char* registro, int tamanioRegistro);
void log_acceso_a_memoria(uint32_t pid, char* modo, uint32_t idSegmento, uint32_t dirFisica, void* valor, uint32_t tamanio);
void loggear_segmentation_fault(uint32_t pid, uint32_t numSegmento, uint32_t offset, uint32_t tamanio);
int obtener_direccion_fisica(PCB *pcb, int dirLogica, int *numeroSegmento, int *offset, int *tamanioSegmento);
int obtener_base_segmento(PCB *pcb, int numeroSegmento,  int *tamanio);

PCB* recibir_pcb(int);
void enviar_pcb_desalojado_a_kernel(PCB*, int, codigo_operacion);
void envio_pcb_a_kernel_con_codigo(int , PCB* , codigo_operacion );

void* get_registro_cpu(char* registro, registros_cpu* registrosCpu);

void iterator(char* value);

pthread_mutex_t m_recibir_pcb;


// LOGS ////////////////////////
#define INSTRUCCION_EJECUTADA        "PID: <%d> - Ejecutando: <INSTRUCCION> - <PARAMETROS>"
#define ACCESO_MEMORIA               "PID: <%d> - Acción: <LEER / ESCRIBIR> - Segmento: <NUMERO SEGMENTO> - Dirección Física: <DIRECCION FISICA> - Valor: <VALOR LEIDO / ESCRITO>"
#define ERROR_SEGMENTATION_FAULT     "PID: <%d> - Error SEG_FAULT- Segmento: <NUMERO SEGMENTO> - Offset: <OFFSET> - Tamaño: <TAMAÑO>"
////////////////////////////////

#define DEFAULT_LOG_PATH      "../../../logs/cpu.log"
#define DEFAULT_CONFIG_PATH   "tuki-pruebas/prueba-memoria/cpu.config"


void devolver_pcb_kernel(PCB*, int, codigo_operacion);

void agregar_registros_a_paquete_para_kernel(t_paquete* , registros_cpu* );

char* agregarCaracterNulo(void* , uint32_t );
int get_int_registro(char* );
int obtener_tamanio_segmento(t_list* lista_segmentos, int numero_segmento);

// uint32_t obtener_direccion_fisica(PCB *pcb,uint32_t dirLogica, uint32_t *numeroSegmento, uint32_t *offset, uint32_t *tamanioSegmento);
void* convertir_dir_logica_a_fisica(PCB *pcb, char* dirLogica);
void* obtener_base_segmento_puntero(PCB *pcb, uint32_t numeroSegmento,  uint32_t *tamanio);
void* obtener_puntero_direccion_fisica(PCB *pcb,uint32_t dirLogica, uint32_t *numeroSegmento, uint32_t *offset, uint32_t *tamanioSegmento);
void loggear_instruccion(PCB* pcb, char** instruccion);

#endif
