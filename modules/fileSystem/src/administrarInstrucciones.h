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

extern t_log* loggerFileSystem;

#define ERROR_ABRIR_ARCHIVO      "No se pudo abrir el archivo: %s"

t_fcb* crear_fcb_inicial(char *nombreArchivo);
bool crear_archivo(char *nombreArchivo);
bool existe_archivo(char *nombreArchivo);
void truncar_archivo(char *nombreArchivo, uint32_t tamanioNuevo);

#endif
