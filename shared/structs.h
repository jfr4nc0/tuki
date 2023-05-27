#ifndef STRUCTS_GLOBAL_H_
#define STRUCTS_GLOBAL_H_

#include <stddef.h>
#include <commons/collections/list.h>

typedef enum {
    OP_PAQUETE,
    OP_EXIT,
    OP_MENSAJE,
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
	int  id; // direccion_base, de que tipo??
	int tamanio;
}t_segmento;


//TODO: POSIBLE CAMBIO
/*
typedef struct {
    // Registros de 4 bytes
    char AX[sizeof(int)];
    char BX[sizeof(int)];
    char CX[sizeof(int)];
    char DX[sizeof(int)];

    // Registros de 8 bytes
    char EAX[sizeof(long)];
    char EBX[sizeof(long)];
    char ECX[sizeof(long)];
    char EDX[sizeof(long)];

    // Registro de 16 bytes
    char RAX[sizeof(long long)];
    char RBX[sizeof(long long)];
    char RCX[sizeof(long long)];
    char RDX[sizeof(long long)];
} registros_cpu;
*/

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
    ENUM_NEW,
    ENUM_READY,
    ENUM_BLOCKED,
    ENUM_EXECUTING,
    ENUM_EXIT,
} pcb_estado;

typedef struct {
	int id_proceso; // Identificador del proceso, unico en todo el sistema
	pcb_estado estado;
	t_list* lista_instrucciones; // Lista de instrucciones a ejecutar
	int contador_instrucciones; // Numero de la proxima instruccion a ejecutar
	registros_cpu* registrosCpu;
	t_list* lista_segmentos;
	t_list* lista_archivos_abiertos; // Contendrá la lista de archivos abiertos del proceso con la posición del puntero de cada uno de ellos.
	float processor_burst; // Estimacion utilizada para planificar los procesos en el algoritmo HRRN, la misma tendra un valor inicial definido por archivo de config y sera recalculada bajo la formula de promedio ponderado
	int ready_timestamp; // Timestamp en que el proceso llegó a ready por última vez (utilizado para el cálculo de tiempo de espera del algoritmo HRRN).
}PCB;

typedef struct {
    //char* IP_KERNEL;
	//char* PUERTO_KERNEL;
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
    char* IP_FILE_SYSTEM;
    char* PUERTO_FILE_SYSTEM;
    char* IP_CPU;
    char* PUERTO_CPU;
    char* PUERTO_ESCUCHA;
    char* ALGORITMO_PLANIFICACION;
    char* ESTIMACION_INICIAL;
    double HRRN_ALFA;
    int GRADO_MAX_MULTIPROGRAMACION;
    char** RECURSOS;
    char** INSTANCIAS_RECURSOS;
} t_kernel_config;

#endif
