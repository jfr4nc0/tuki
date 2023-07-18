#ifndef FILE_SYSTEM_ESTRUCTURAS_H_
#define FILE_SYSTEM_ESTRUCTURAS_H_

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <commons/bitarray.h>
#include <commons/config.h>

#include "compartido.h"

#include <shared/shared.h>

typedef struct {
	int RETARDO_ACCESO_BLOQUE;
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	int PUERTO_ESCUCHA;
    char* PATH_FCB;
    char* PATH_BITMAP;
    char* PATH_SUPERBLOQUE;
    char* PATH_BLOQUES;
}t_config_file_system;

typedef struct {
    uint32_t block_size;
    uint32_t block_count;
}t_superbloque;

typedef struct {
    char* direccion;
    uint32_t size;
    t_bitarray* bitarray;
}t_bitmap;

typedef struct {
    char* nombre_archivo; // Funciona como id archivo
    uint32_t tamanio_archivo; // En bytes
    uint32_t puntero_directo; // Primer bloque
    uint32_t puntero_indirecto; // Siguientes bloques
    uint32_t cantidad_bloques_asignados;
}t_fcb;

extern t_log* loggerFileSystem;
extern t_config_file_system* configFileSystem;



void inicializar_estructuras(t_config* config);

void cargar_config(t_config* config);
t_superbloque* crear_superbloque (char *pathSuperbloque);
void cargar_config(t_config* config);
t_bitarray* create_bitmap(int entries);
void abrir_bitmap (char* pathBitmap, uint32_t blockCount);
void crear_archivo_de_bloques(char* pathArchivoDeBloques, uint32_t blockCount, uint32_t blockSize);
void abrir_fcbs(char* path_fcbs);

#endif
