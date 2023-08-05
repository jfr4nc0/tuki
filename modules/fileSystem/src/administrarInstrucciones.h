#ifndef ADMINISTRAR_FILE_SYSTEM_ADMINISTRAR_INSTRUCCIONES_H_
#define ADMINISTRAR_FILE_SYSTEM_ADMINISTRAR_INSTRUCCIONES_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <commons/collections/dictionary.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <dirent.h>
#include <limits.h>

#include <shared/shared.h>
#include "inicializarEstructuras.h"
#include "administrarEstructuras.h"
#include "constantes.h"


extern t_log* loggerFileSystem;

t_fcb* crear_fcb_inicial(char *nombreArchivo);
bool crear_archivo(char *nombreArchivo);
bool existe_archivo(char *nombreArchivo);
bool truncar_archivo(char *nombreArchivo, uint32_t tamanioNuevo);
char* leer_archivo(char* nombreArchivo, uint32_t puntero, int direccionFisica, uint32_t bytesQueFaltanPorLeer, uint32_t pidProceso);
bool escribir_archivo(char* informacionAEscribir, char *nombreArchivo, uint32_t puntero, uint32_t cantidadBytesAEscribir, void* direccionFisica);

// Auxiliares
uint32_t obtener_posicion_en_bloque(uint32_t punteroFseek);
uint32_t espacio_disponible_en_bloque_desde_posicion(uint32_t punteroFseek);
uint32_t obtener_bloque_relativo(t_fcb* fcb, uint32_t punteroFseek);
uint32_t obtener_bloque_absoluto(t_fcb* fcb, uint32_t punteroFseek) ;
uint32_t obtener_posicion_absoluta(t_fcb* fcb, uint32_t punteroFseek);
void escribir_en_bloque(uint32_t posicion, uint32_t cantidadBytesAEscribir, char *informacionAEscribir);
uint32_t buscar_siguiente_bloque(uint32_t bloqueActual, t_fcb *fcbArchivo);

#endif
