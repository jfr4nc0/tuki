#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

// Externas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>

// Internas
#include "compartido.h"
#include "administrarFileSystem.h"
#include "estructuras.h"
#include <shared/shared.h>

t_log* loggerFileSystem;

t_list* listaFCB;

// Paths
#define DEFAULT_LOG_PATH        "logs/file_system.log"
#define DEFAULT_CONFIG_PATH     "tuki-pruebas/prueba-base/fileSystem.config"
#define CONFIG_PROPIO_PATH      "tuki-pruebas/prueba-base/fileSystem_propio.config"

//// LOGS
#define ACCESO_BITMAP            "Acceso a Bitmap - Bloque: <NUMERO BLOQUE> - Estado: <ESTADO>" Nota: El estado es 0 o 1 donde 0 es libre y 1 es ocupado.
#define ACCESO_BLOQUE            "Acceso Bloque - Archivo: <NOMBRE_ARCHIVO> - Bloque Archivo: <NUMERO BLOQUE ARCHIVO> - Bloque File System <NUMERO BLOQUE FS>"
#define APERTURA_ARCHIVO         "Abrir Archivo: <NOMBRE_ARCHIVO>"#define CREAR ARCHIVO               "Crear Archivo: <NOMBRE_ARCHIVO>"
#define ESCRITURA_ARCHIVO        "Escribir Archivo: <NOMBRE_ARCHIVO> - Puntero: <PUNTERO ARCHIVO> - Memoria: <DIRECCION MEMORIA> - Tamaño: <TAMAÑO>"
#define LECTURA_ARCHIVO          "Leer Archivo: <NOMBRE_ARCHIVO> - Puntero: <PUNTERO ARCHIVO> - Memoria: <DIRECCION MEMORIA> - Tamaño: <TAMAÑO>"
#define TRUNCATE_ARCHIVO         "Truncar Archivo: <NOMBRE_ARCHIVO> - Tamaño: <TAMAÑO>"
/////////

// Functions
void atender_kernel(int);
void ejecutar_instrucciones_kernel(void*);
void iterator(char* value);

#endif
