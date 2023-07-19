#ifndef SHARED_H
#define SHARED_H

// Externas
#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
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

/*--------------------------------- Estructuras --------------------------------*/

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
    AUX_CREATE_PCB,
	// Desalojos
	DESALOJO_YIELD,
    // Auxiliares
	AUX_MENSAJE,
	AUX_OK,
	AUX_ERROR,
    AUX_SOLO_CON_COMPACTACION,
	AUX_NEW_PROCESO, // Notifica a kernel que hay un nuevo proceso y se le envia la lista de instrucciones
	AUX_SOY_CPU, // Notifica a memoria que el modulo que se conectó es CPU
	AUX_SOY_KERNEL, // Notifica a memoria que el modulo que se conectó es KERNEL
	AUX_SOY_FILE_SYSTEM // Notifica a memoria que el modulo que se conectó es FILE SYSTEM
}codigo_operacion;
typedef struct {
    // Registros de 4 bytes
    char* AX;
    char* BX;
    char* CX;
    char* DX;

    // Registros de 8 bytes
    char* EAX;
    char* EBX;
    char* ECX;
    char* EDX;

    // Registro de 16 bytes
    char* RAX;
    char* RBX;
    char* RCX;
    char* RDX;
} registros_cpu;

typedef struct {
    int size;
    void* stream;
}t_buffer;

typedef struct {
    codigo_operacion codigoOperacion;
    t_buffer* buffer;
}t_paquete;

typedef struct {
    int  id; // File descriptor
    int posicion_puntero;
} archivo_abierto_t;

/*--------------------------------- FUNCIONES GENERALES --------------------------------*/

char* cantidad_strings_a_mostrar(int);
char* extraer_string_de_config(t_config*, char*, t_log* logger);
int extraer_int_de_config(t_config* config, char* property, t_log* logger);
char* extraer_de_modulo_config(t_config*, char*, char*, t_log* logger);
char* concatenar_strings(char*, char*);
t_log* iniciar_logger(char*, int);
t_config* iniciar_config(char*, t_log*);
void terminar_programa(int, t_log*, t_config*);
void liberar_conexion(int);
bool obtener_valores_para_logger(int, bool*, t_log_level*, char**);
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

/*----------------------------- FUNCIONES SERVIDOR ----------------------------*/

int iniciar_servidor(t_config*, t_log*);
int esperar_cliente(int, t_log*);
t_list* recibir_paquete(int);
int recibir_operacion(int);
void* recibir_buffer(int*, int);
void* leer_de_buffer(char*, int*, size_t);

void intervalo_de_pausa(int );

#endif
