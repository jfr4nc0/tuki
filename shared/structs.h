#ifndef STRUCTS_GLOBAL_H_
#define STRUCTS_GLOBAL_H_

#include <stddef.h>

typedef enum
{
    MENSAJE,
    PAQUETE
}op_code;

typedef struct
{
    int size;
    void* stream;
}t_buffer;

typedef struct
{
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

/* TODO: Hacer enum para iniciar_log
enum modulos {
    CONSOLA = ENUM_CONSOLA,
    CPU = ENUM_CPU,
    FILE_SYSTEM = ENUM_FILE_SYSTEM,
    KERNEL = ENUM_KERNEL,
    MEMORIA = ENUM_MEMORIA
};
*/

#endif
