#ifndef ADMINISTRAR_FILE_SYSTEM_ADMINISTRAR_ESTRUCTURAS_H_
#define ADMINISTRAR_FILE_SYSTEM_ADMINISTRAR_ESTRUCTURAS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/config.h>
#include <dirent.h>
#include <limits.h>

#include <shared/shared.h>
#include "inicializarEstructuras.h"
#include "constantes.h"

bool actualizar_tamanio_bloques(t_fcb* fcbArchivo, uint32_t tamanioNuevo);
void ampliar_archivo(t_fcb *fcbArchivo, uint32_t tamanioNuevo);
void asignar_bloques_archivo_vacio(t_fcb *fcbArchivo,uint32_t tamanioNuevo);
void asignar_bloques_archivo_con_informacion(t_fcb *fcbArchivo,uint32_t tamanioNuevo);
void asignar_puntero_directo(t_fcb *fcbArchivo);
void reducir_archivo(t_fcb *fcbArchivo, uint32_t tamanioNuevo);
void bitmap_marcar_bloque_ocupado(uint32_t numeroBloque);
void asignar_puntero_indirecto(t_fcb *fcbArchivo);
void vaciar_archivo(t_fcb *fcbArchivo);
bool persistir_fcb(t_fcb* fcb);
int32_t bitmap_encontrar_bloque_libre();
bool asignar_bloques(t_fcb *fcbArchivo, uint32_t cantidadBloques);
void desasignar_bloques(t_fcb *fcbArchivo, uint32_t cantidadBloquesDesasignar);
bool desasignar_ultimo_bloque(t_fcb *fcbArchivo);
int32_t leer_ultimo_puntero_de_bloque_de_punteros(t_fcb* fcb);
int32_t archivo_de_bloques_leer_n_puntero_de_bloque_de_punteros(uint32_t bloque, uint32_t punteroN);

#endif
