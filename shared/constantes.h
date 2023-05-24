#ifndef CONSTANTES_GLOBALES_H_
#define CONSTANTES_GLOBALES_H_

#include "configuraciones.h"

#define LOCALHOST           "127.0.0.1"
#define PUERTO_LOCAL        "PUERTO_ESCUCHA"

// Signos
#define ENTER             "\n"
#define SIGN_CONSOLA      "> "
#define EMPTY_STRING      ""

// CONSTANTES
#define MODO_LECTURA_ARCHIVO      "r"
#define IP_CONFIG                 "IP_"
#define PUERTO_CONFIG             "PUERTO_"
#define LONGITUD_MAXIMA_CADENA    1000
#define CANTIDAD_ESTADOS          5

// Modulos
#define CONSOLA                    "CONSOLA"
#define CPU                        "CPU"
#define FILE_SYSTEM                "FILE_SYSTEM"
#define KERNEL                     "KERNEL"
#define MEMORIA                    "MEMORIA"

// ENUMS

#define ENUM_CONSOLA              	0
#define ENUM_CPU                    1
#define ENUM_FILE_SYSTEM            2
#define ENUM_KERNEL               	3
#define ENUM_MEMORIA              	4

#define NEW                             "NEW"
#define READY                           "READY"
#define BLOCKED                         "BLOCKED"
#define EXECUTING                       "EXECUTING"
#define EXIT                            "EXIT"
#define IO                              "I0"

// DEBUG MENSAJES
#define D__ESTABLECIENDO_CONEXION   "Estableciendo conexion"
#define D__CONFIG_CREADO            "Config creado"
#define D__LOG_CREADO               "Log creado"

// INFO MENSAJES
#define I__CONEXION_CREATE          "Conexion creada"
#define I__DESCONEXION_CLIENTE      "El cliente se desconecto. Terminando servidor"
#define I__SERVER_READY             "Servidor listo para recibir al cliente:"


// ERROR MENSAJES
#define E__ARCHIVO_CREATE      "Error al crear/leer archivo"
#define E__BAD_REQUEST         "BAD REQUEST"
#define E__CONEXION_CREATE     "Error al crear conexion"
#define E__CONEXION_CONNECT    "Error al conectar conexion"
#define E__LOGGER_CREATE       "No se pudo crear logger"
#define E__CONFIG_CREATE       "No se pudo crear config"
#define E__PAQUETE_CREATE      "Error al crear paquete"

const char* nombres_estados[] = {
        NEW,
        READY,
        BLOCKED,
        EXECUTING,
        EXIT
};

#endif
