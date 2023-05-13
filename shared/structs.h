#ifndef STRUCTS_GLOBAL_H_
#define STRUCTS_GLOBAL_H_

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
