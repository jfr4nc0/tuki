#ifndef FILE_SYSTEM_CONSTANTES_H_
#define FILE_SYSTEM_CONSTANTES_H_

#define DEFAULT_LOG_PATH "logs/fileSystem.log"
#define DEFAULT_CONFIG_PATH "fileSystem.config"

//// LOGS
#define ACCESO_BITMAP            "Acceso a Bitmap - Bloque: <NUMERO BLOQUE> - Estado: <ESTADO>" Nota: El estado es 0 o 1 donde 0 es libre y 1 es ocupado.
#define ACCESO_BLOQUE            "Acceso Bloque - Archivo: <NOMBRE_ARCHIVO> - Bloque Archivo: <NUMERO BLOQUE ARCHIVO> - Bloque File System <NUMERO BLOQUE FS>"
#define APERTURA_ARCHIVO         "Abrir Archivo: <NOMBRE_ARCHIVO>"#define CREAR ARCHIVO               "Crear Archivo: <NOMBRE_ARCHIVO>"
#define ESCRITURA_ARCHIVO        "Escribir Archivo: <NOMBRE_ARCHIVO> - Puntero: <PUNTERO ARCHIVO> - Memoria: <DIRECCION MEMORIA> - Tamaño: <TAMAÑO>"
#define LECTURA_ARCHIVO          "Leer Archivo: <NOMBRE_ARCHIVO> - Puntero: <PUNTERO ARCHIVO> - Memoria: <DIRECCION MEMORIA> - Tamaño: <TAMAÑO>"
#define TRUNCATE_ARCHIVO         "Truncar Archivo: <NOMBRE_ARCHIVO> - Tamaño: <TAMAÑO>"
/////////

#endif
