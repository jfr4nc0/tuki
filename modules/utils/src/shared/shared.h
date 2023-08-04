#ifndef SHARED_H
#define SHARED_H

// Externas
#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include <commons/string.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <string.h>

// Internas
#include "constantes.h"

extern char* nombres_estados[5];
typedef enum {
    ENUM_CONSOLA,
    ENUM_CPU,
    ENUM_FILE_SYSTEM,
    ENUM_KERNEL,
    ENUM_MEMORIA
}enum_modulos;

typedef enum {
    ENUM_NEW,
    ENUM_READY,
    ENUM_EXECUTING,
    ENUM_BLOCKED,
    ENUM_EXIT,
}pcb_estado;

typedef enum {
    ENUM_ARCHIVO_FREE,
    ENUM_ARCHIVO_BLOCK,
}t_nombre_estado;

/*--------------------------------- Estructuras --------------------------------*/

typedef struct {
    // Registros de 4 bytes
    char AX[4];
    char BX[4];
    char CX[4];
    char DX[4];

    // Registros de 8 bytes
    char EAX[8];
    char EBX[8];
    char ECX[8];
    char EDX[8];

    // Registro de 16 bytes
    char RAX[16];
    char RBX[16];
    char RCX[16];
    char RDX[16];
} registros_cpu;

typedef struct {
	int id_proceso; // Identificador del proceso, unico en todo el sistema
	pcb_estado estado;
	t_list* lista_instrucciones; // Lista de instrucciones a ejecutar
	int contador_instrucciones; // Numero de la proxima instruccion a ejecutar
	registros_cpu* registrosCpu;
	t_list* lista_segmentos;
	t_list* lista_archivos_abiertos; // Contendrá la lista de archivos abiertos del proceso con la posición del puntero de cada uno de ellos.
	double estimacion_rafaga; // Estimacion utilizada para planificar los procesos en el algoritmo HRRN, la misma tendra un valor inicial definido por archivo de config y sera recalculada bajo la formula de promedio ponderado
	double ready_timestamp; // Timestamp en que el proceso llegó a ready por última vez (utilizado para el cálculo de tiempo de espera del algoritmo HRRN).
}PCB;

typedef enum {
    OP_EXECUTE_PCB,
    // Instrucciones
    I_SET,
    I_MOV_IN,
    I_MOV_OUT,
    I_IO,
    I_F_OPEN,
    I_F_CLOSE,
    I_F_SEEK,
    I_F_READ,
    I_F_WRITE,
    I_TRUNCATE,
    I_WAIT,
    I_SIGNAL,
    I_CREATE_SEGMENT,
    I_DELETE_SEGMENT,
    I_YIELD,
    I_EXIT,
	SEGMENTATION_FAULT,
    AUX_CREATE_PCB,
	// Desalojos
	DESALOJO_YIELD,
	DESALOJO_EXIT,
	I_DESCONOCIDA,
	EXIT__SUCCESS,
	WAIT_RECURSO_NO_EXISTENTE,
	SIGNAL_RECURSO_NO_EXISTENTE,
	EXIT_SEGMENTATION_FAULT,
	TERMINAR_EJECUCION,
    // Kernel
    KERNEL_CREAR_ARCHIVO,
    // Auxiliares
	AUX_MENSAJE,
	AUX_OK,
	AUX_ERROR,
    AUX_FINALIZAR_PROCESO,
	AUX_NEW_PROCESO, // Notifica a kernel que hay un nuevo proceso y se le envia la lista de instrucciones
	AUX_SOY_CPU, // Notifica a memoria que el modulo que se conectó es CPU
	AUX_SOY_KERNEL, // Notifica a memoria que el modulo que se conectó es KERNEL
	AUX_SOY_FILE_SYSTEM,
    AUX_PID,
    AUX_NUEVO_SEGMENTO,
    AUX_PERMISOS_INSUFICIENTES,
	SEGMENTO_CREADO,
	OUT_OF_MEMORY,
	COMPACTACION,
	AX,
	BX,
	CX,
	DX,
	EAX,
	EBX,
	ECX,
	EDX,
	RAX,
	RBX,
	RCX,
	RDX,
}codigo_operacion;

typedef struct {
    int size;
    void* stream;
}t_buffer;

typedef struct {
    codigo_operacion codigoOperacion;
    t_buffer* buffer;
}t_paquete;

typedef struct {
    void* direccionBase;
    size_t size;
    int id;
} t_segmento;

/*
typedef struct{
	int id;
	void* direccion_base;
	int tamanio_segmento;
}segmento_t;
*/

typedef struct {
    int PID;
    t_list* segmentos;
} t_tabla_segmentos;

typedef struct {
    t_segmento* segmento;
    int idProceso;
} t_segmento_tabla;

typedef struct {
	int id_proceso;
	t_list* lista_segmentos;
} t_proceso_actualizado;

typedef struct {
    char* nombreArchivo;
    uint32_t puntero;
}t_archivo_abierto;

typedef struct {
    char* nombreArchivo;
    uint32_t puntero;
}t_enviar_archivo_abierto;
/*--------------------------------- FUNCIONES GENERALES --------------------------------*/
void* calcular_direccion(void* posicionBase, size_t desplazamiento);
void intervalo_de_pausa(int );
char** decode_instruccion(char*, t_log*);
char* encode_instruccion(char** strings);
char* truncar_string(char* str,int size);
char* cantidad_strings_a_mostrar(int);
char* extraer_string_de_config(t_config*, char*, t_log* logger);
int extraer_int_de_config(t_config* config, char* property, t_log* logger);
char* extraer_de_modulo_config(t_config*, char*, char*, t_log* logger);
char* concatenar_strings(char*, char*);
bool obtener_valores_para_logger(int, bool*, t_log_level*, char**);
void mostrarListaSegmentos(t_list* segmentos);
void mostrar_pcb(PCB* pcb, t_log* logger);
void iteratorSinLog(char* value);

void serializar_tabla_segmentos(t_list *tabla_segmentos, t_paquete *paquete);

t_list* deserializar_tabla_segmentos(void* buffer, int* desplazamiento);
t_list* deserealizar_todas_las_tablas_segmentos(void* buffer, int* desplazamiento);

void serializar_todas_las_tablas_segmentos(t_list* tablas_segmentos, t_paquete* paquete);

long leer_long(char* buffer, int* desp);
long long leer_long_long(char* buffer, int* desp);
float leer_float(char* buffer, int* desp);
int leer_int(char* buffer, int* desp);
double leer_double(char*, int*);
char* leer_string(char* buffer, int* desp);
t_list* leer_string_array(char* buffer, int* desp);
char** leer_arreglo_string(char* , int* );
char* leer_registro_4_bytes(char* , int* );
char* leer_registro_8_bytes(char* , int* );
char* leer_registro_16_bytes(char* , int* );

void agregar_lista_a_paquete(t_paquete* paquete, t_list* lista, t_log* logger);

void agregar_registro4bytes_a_paquete(t_paquete* paquete, char valor[4]);
void agregar_registro8bytes_a_paquete(t_paquete* paquete, char valor[8]);
void agregar_registro16bytes_a_paquete(t_paquete* paquete, char valor[16]);
void agregar_registros_a_paquete(t_paquete* paquete, registros_cpu* registrosCpu);

/*--------- BUFFERS ------------*/
void buffer_pack(t_buffer* self, void* streamToAdd, int size);
void __stream_send(int toSocket, void *streamToSend, uint32_t bufferSize);
t_buffer *buffer_unpack(t_buffer *self, void *dest, int size);
t_buffer *buffer_create(void);
void *__stream_create(uint8_t header, t_buffer *buffer);
void stream_send_buffer(int toSocket, uint8_t header, t_buffer *buffer);
char *buffer_unpack_string(t_buffer *self);
void buffer_pack_string(t_buffer *self, char *stringToAdd);

uint32_t leer_uint32(char* buffer, int* desp);
t_list* recibir_lista_segmentos(int cliente);
void agregar_registro_a_paquete(t_paquete* paquete, char* registro, int tamanio_registro);
char* leer_registro_de_buffer(char* buffer, int desplazamiento);

t_list* recibir_resto_lista_segmentos(void* buffer, int* desp);
void agregar_lista_segmentos_a_paquete(t_paquete* buffer, int cliente, t_list* segmentosTabla, t_log* logger);

void* recibir_puntero(int clienteAceptado);

void agregar_lista_segmentos_del_proceso(t_paquete* paquete, int cliente, t_list* segmentosTabla, t_log* logger);
void enviar_segmento_por_pid(int cliente, codigo_operacion,t_segmento_tabla* tabla_segmento);
t_segmento_tabla* recibir_segmento_por_pid(int cliente);

void enviar_lista_segmentos_del_proceso(int cliente, t_list* segmentosTabla, t_log* logger);

void enviar_pcb(int conexion, PCB* pcb_a_enviar, codigo_operacion codigo, t_log* log);

t_log* iniciar_logger(char*, int);
t_config* iniciar_config(char*, t_log*);
void terminar_programa(int, t_log*, t_config*);
void liberar_conexion(int);

void agregar_size_a_paquete(t_paquete* paquete, size_t valor);
void agregar_valor_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void agregar_lista_archivos_a_paquete(t_paquete* paquete, t_list* lista, t_log* logger);
void agregar_pcb_a_paquete(t_paquete* paquete, PCB* pcb, t_log* log);

PCB* recibir_pcb(int);

void agregar_int_a_paquete(t_paquete* paquete, int valor);
uint32_t agregar_uint32_a_paquete(t_paquete* paquete, uint32_t valor);
void* leer_algo(char* buffer, int* desp, size_t size);
size_t leer_size(char* buffer, int* desp);


/*----------------------------- FUNCIONES CLIENTE ----------------------------*/

int crear_conexion(char*, char*, char*, t_log*);
void enviar_mensaje(char*, int, t_log*);
t_paquete* crear_paquete(codigo_operacion);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete*, void*, size_t);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
int armar_conexion(t_config*, char*, t_log*);
void enviar_operacion(int conexion, codigo_operacion, size_t tamanio, void* valor);
void enviar_codigo_operacion(int, codigo_operacion);

void enviar_tabla_segmentos(int conexion, codigo_operacion codOperacion, t_list* tabla_segmento);
void enviar_nuevo_segmento(int cliente, t_segmento* segmento);
t_list* desempaquetar_tabla_segmentos(t_buffer *bufferTablaSegmentos, uint32_t tamanioTablaSegmentos);
t_buffer* empaquetar_tabla_segmentos(t_list* tablaSegmentos, uint32_t tamanioTablaSegmentos);
void stream_recv_buffer(int fromSocket, t_buffer *destBuffer);

void enviar_msj_con_parametros(int socket, int op_code, char** parametros);

/*----------------------------- FUNCIONES SERVIDOR ----------------------------*/

int iniciar_servidor(t_config*, t_log*);
int esperar_cliente(int, t_log*);
t_list* recibir_paquete(int);
int recibir_operacion(int);
void* recibir_buffer(int*, int);
void* leer_de_buffer(char*, int*, size_t);
char* leer_texto(char* buffer, int* desp, int size);

void* leer_puntero(void* buffer, int* desp);
void agregar_puntero_a_paquete(t_paquete* paquete, void* valor);

//timestamp* leer_timestamp(char* buffer, int* desp);

void intervalo_de_pausa(int );


char** decode_instruccion(char*, t_log*);

#endif
