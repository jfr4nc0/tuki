#ifndef SHARED_H
#define SHARED_H

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


typedef struct {
    // Registros de 4 bytes
    int AX;
    int BX;
    int CX;
    int DX;

    // Registros de 8 bytes
    long EAX;
    long EBX;
    long ECX;
    long EDX;

    // Registro de 16 bytes
    long long RAX;
    long long RBX;
    long long RCX;
    long long RDX;
} registros_cpu;

typedef enum {
	OP_EXIT,
	OP_MENSAJE,
	OP_YIELD,
	OP_CREATE_SEGMENT,
	OP_DELETE_SEGMENT,
	AUX_NEW_PROCESO, // Notifica a kernel que hay un nuevo proceso y se le envia la lista de instrucciones
	AUX_SOY_CPU, // Notifica a memoria que el modulo que se conectó es CPU
	AUX_SOY_KERNEL, // Notifica a memoria que el modulo que se conectó es KERNEL
	AUX_SOY_FILE_SYSTEM, // Notifica a memoria que el modulo que se conectó es FILE SYSTEM
	OP_EXECUTE_PCB
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
    int  id; // File descriptor
    int posicion_puntero;
} archivo_abierto_t;

typedef struct {
	int  id;
	int tamanio;
}t_segmento;

typedef struct {
    t_segmento* segmentos;
    int cantidad_segmentos_usados;
    int capacidad_segmentos;
}t_tabla_segmentos;


/*---------------------------------- INSTRUCTIONS ----------------------------------*/

#define BADKEY -1
#define I_SET 1
#define I_MOV_IN 2
#define I_MOV_OUT 3
#define I_IO 4
#define I_F_OPEN 5
#define I_F_CLOSE 6
#define I_F_SEEK 7
#define I_F_READ 8
#define I_F_WRITE 9
#define I_TRUNCATE 10
#define I_WAIT 11
#define I_SIGNAL 12
#define I_CREATE_SEGMENT 13
#define I_DELETE_SEGMENT 14
#define I_YIELD 15
#define I_EXIT 16

int keyfromstring(char *key);

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
char* leer_string(char* buffer, int* desp);
t_list* leer_string_array(char* buffer, int* desp);
char** leer_arreglo_string(char* , int* );


/*----------------------------- FUNCIONES CLIENTE ----------------------------*/

int crear_conexion(char*, char*, t_log*);
void enviar_mensaje(char*, int, t_log*);
t_paquete* crear_paquete(codigo_operacion);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete*, void*, int);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
int armar_conexion(t_config*, char*, t_log*);
void enviarOperacion(int conexion, codigo_operacion, int tamanio, void* valor);
//void devolver_pcb_kernel(PCB*, int, codigo_operacion);
void identificarse(int, codigo_operacion);

/*----------------------------- FUNCIONES SERVIDOR ----------------------------*/

int iniciar_servidor(t_config*, t_log*);
int esperar_cliente(int, t_log*);
t_list* recibir_paquete(int);
int recibir_operacion(int);
void* recibir_buffer(int*, int);

/*------------------------------ CONFIGURACIONES ------------------------------*/

#define PATH_DEFAULT_CONEXION_KERNEL                "../../tuki-pruebas/prueba-base/kernel_conexion.config"

#define MOSTRAR_OCULTAR_MENSAJES_LOG_CONSOLA        1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_CPU            1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_FILE_SYSTEM    1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_MEMORIA        1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL         1

#define LOG_LEVEL_CONSOLA                           LOG_LEVEL_TRACE
#define LOG_LEVEL_CPU                               LOG_LEVEL_TRACE
#define LOG_LEVEL_FILE_SYSTEM                       LOG_LEVEL_TRACE
#define LOG_LEVEL_KERNEL                            LOG_LEVEL_TRACE
#define LOG_LEVEL_MEMORIA                           LOG_LEVEL_TRACE

/*
 * Si se quiere cambiar todos los modulos a la vez se deberia poder
 * setear este valor y mover los ENU_<MODULO> de constantes.h a un numero mayor a 4
 */
#define LOG_LEVEL_DEFAULT         LOG_LEVEL_INFO

/*--------------------------------- CONSTANTES --------------------------------*/

#define LOCALHOST           "127.0.0.1"
#define PUERTO_LOCAL        "PUERTO_ESCUCHA"
#define ERROR               "ERROR"
#define OK                  "OK"
#define HANDSHAKE           "HANDSHAKE"

// Signos
#define ENTER             "\n"
#define SIGN_CONSOLA      "> "
#define EMPTY_STRING      ""

// CONSTANTES
#define MODO_LECTURA_ARCHIVO      "r"
#define IP_CONFIG                 "IP_"
#define PUERTO_CONFIG             "PUERTO_"
#define LONGITUD_MAXIMA_CADENA    1000
#define CANTIDAD_ESTADOS          5

// Modulos
#define CONSOLA                    "CONSOLA"
#define CPU                        "CPU"
#define FILE_SYSTEM                "FILE_SYSTEM"
#define KERNEL                     "KERNEL"
#define MEMORIA                    "MEMORIA"

// ENUMS
#define ENUM_CONSOLA              	0
#define ENUM_CPU                    1
#define ENUM_FILE_SYSTEM            2
#define ENUM_KERNEL               	3
#define ENUM_MEMORIA              	4

#define NEW                             "NEW"
#define READY                           "READY"
#define BLOCKED                         "BLOCKED"
#define EXECUTING                       "EXECUTING"
#define EXIT                            "EXIT"
#define IO                              "I0"

// DEBUG MENSAJES
#define D__ESTABLECIENDO_CONEXION   "Estableciendo conexion"
#define D__CONFIG_INICIAL_CREADO    "Config creado"
#define D__LOG_CREADO               "Log creado"

// INFO MENSAJES
#define I__CONEXION_CREATE          "Conexion creada"
#define I__CONEXION_ACCEPT          "Se conecto un cliente"
#define I__DESCONEXION_CLIENTE      "El cliente se desconecto. Terminando servidor"
#define I__SERVER_READY             "Servidor listo para recibir al cliente: "
#define I_ESPERANDO_CONEXION        "Esperando conexiones..."
#define I__CONFIG_GENERIDO_CARGADO  "Config generico creado: %s"

// ERROR MENSAJES
#define E__ARCHIVO_CREATE      "Error al crear/leer archivo"
#define E__BAD_REQUEST         "BAD REQUEST"
#define E__CONEXION_CREATE     "Error al crear conexion"
#define E__CONEXION_CONNECT    "Error al conectar conexion"
#define E__CONEXION_ACEPTAR    "Error al aceptar conexion"
#define E__LOGGER_CREATE       "No se pudo crear logger"
#define E__CONFIG_CREATE       "No se pudo crear config"
#define E__PAQUETE_CREATE      "Error al crear paquete"
#define E__MALLOC_ERROR        "Error al crear el malloc de tamaño %d "


#endif
