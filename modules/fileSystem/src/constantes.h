#ifndef CONSTANTES_FILE_SYSTEM_H_
#define CONSTANTES_FILE_SYSTEM_H_

// Paths
#define DEFAULT_CONFIG_PATH     "tuki-pruebas/prueba-memoria/fileSystem.config"
#define CONFIG_PROPIO_PATH      "tuki-pruebas/prueba-base/fileSystem_propio.config"

//// LOGS
#define ERROR_ABRIR_ARCHIVO                     "No se pudo abrir el archivo: %s"
#define LEER_ARCHIVO                            "Leer Archivo: <%s> - Puntero: <%u> - Memoria: <%p> - Tamaño: <%u>"
#define FCB_NOT_FOUND                           "No se encontró el fcb en la lista de fcbs. Nombre archivo: %s"
#define MODIFICADO_BITMAP                       "Acceso a Bitmap - Bloque: <%u> - Estado: <%u>"
#define LOG_ACCESO_BLOQUE                       "Acceso Bloque - Archivo: <%s> - Bloque Archivo: <%u> - Bloque File System <%u>"
#define LOG_BLOQUE_DESASIGNADO                  "Bloque <%u> desasignado al Archivo: <%s>"
#define LOG_PUNTERO_DIRECTO_DESASIGNADO         "Puntero Directo Desasignado"
#define LOG_PUNTERO_INDIRECTO_DESASIGNADO       "Bloque de punteros desasignado"

#define ACCESO_BITMAP            "Acceso a Bitmap - Bloque: <NUMERO BLOQUE> - Estado: <ESTADO>"
#define ACCESO_BLOQUE            "Acceso Bloque - Archivo: <%s> - Bloque Archivo: <%u> - Bloque File System <%u>"
#define APERTURA_ARCHIVO         "Abrir Archivo: <NOMBRE_ARCHIVO>"
#define CREAR_ARCHIVO            "Crear Archivo: <NOMBRE_ARCHIVO>"
#define ESCRIBIR_ARCHIVO         "Escribir Archivo: <%s> - Puntero: <%u> - Memoria: <%u> - Tamaño: <%u>"
#define TRUNCATE_ARCHIVO         "Truncar Archivo: %s - Tamaño: <%u>"
/////////

typedef enum {
    ABAJO,
    ARRIBA
}codigo_redondear;

#endif
