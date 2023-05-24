#ifndef STRUCTS_GLOBAL_H_
#define STRUCTS_GLOBAL_H_

#include <stddef.h>
#include<commons/collections/list.h>

typedef enum {
    MENSAJE,
    PAQUETE
}op_code;

typedef struct {
    int size;
    void* stream;
}t_buffer;

typedef struct {
    op_code codigo_operacion;
    t_buffer* buffer;
}t_paquete;

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
} cpu_registers;

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
	int program_counter; // Numero de la proxima instruccion a ejecutar
	cpu_registers* cpu_register;
	t_list* lista_segmentos;
	t_list* lista_archivos_abiertos; // Contendrá la lista de archivos abiertos del proceso con la posición del puntero de cada uno de ellos.
	float processor_burst ; // Estimacion utilizada para planificar los procesos en el algoritmo HRRN, la misma tendra un valor inicial definido por archivo de config y sera recalculada bajo la formula de promedio ponderado
	int ready_timestamp; // Timestamp en que el proceso llegó a ready por última vez (utilizado para el cálculo de tiempo de espera del algoritmo HRRN).
}PCB;

#endif
