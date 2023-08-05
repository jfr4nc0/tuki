#ifndef SHARED_CONSTANTES_H
#define SHARED_CONSTANTES_H

/*---------------------------------- INSTRUCTIONS ----------------------------------*/
/* Puesto dentro de codigo_operacion tipo
#define BAD_KEY -1
#define I_SET 1
#define I_MOV_IN 2
#define I_MOV_OUT 3
#define I_IO 4
#define I_F_OPEN 5
#define I_F_CLOSE 6
#define I_F_SEEK 7
#define I_F_READ 8
#define I_F_WRITE 9
#define I_TRUNCATE 10
#define I_WAIT 11
#define I_SIGNAL 12
#define I_CREATE_SEGMENT 13
#define I_DELETE_SEGMENT 14
#define I_YIELD 15
#define I_EXIT 16
*/
int keyFromString(char *key);

/*------------------------------ CONFIGURACIONES ------------------------------*/

#define PATH_DEFAULT_CONEXION_KERNEL                "tuki-pruebas/prueba-base/consola.config"

#define MOSTRAR_OCULTAR_MENSAJES_LOG_CONSOLA        1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_CPU            1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_FILE_SYSTEM    1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_MEMORIA        1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL         1

#define LOG_LEVEL_CONSOLA                           LOG_LEVEL_TRACE
#define LOG_LEVEL_CPU                               LOG_LEVEL_TRACE
#define LOG_LEVEL_FILE_SYSTEM                       LOG_LEVEL_TRACE
#define LOG_LEVEL_KERNEL                            LOG_LEVEL_TRACE
#define LOG_LEVEL_MEMORIA                           LOG_LEVEL_TRACE

/*
 * Si se quiere cambiar todos los modulos a la vez se deberia poder
 * setear este valor y mover los ENU_<MODULO> de constantes.h a un numero mayor a 4
 */
#define LOG_LEVEL_DEFAULT         LOG_LEVEL_INFO

/*--------------------------------- CONSTANTES --------------------------------*/

#define LOCALHOST           "127.0.0.1"
#define PUERTO_LOCAL        "PUERTO_ESCUCHA"
#define ERROR               "ERROR"
#define OK                  "OK"
#define HANDSHAKE           "HANDSHAKE %s"

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

#define NEW                             "NEW"
#define READY                           "READY"
#define BLOCKED                         "BLOCKED"
#define EXECUTING                       "EXECUTING"
#define EXIT                            "EXIT"
#define IO                              "I0"

// DEBUG MENSAJES
#define D__ESTABLECIENDO_CONEXION   "Estableciendo conexion con %s"
#define D__CONFIG_INICIAL_CREADO    "Config creado"
#define D__LOG_CREADO               "Log creado"
#define D__LOG_SEGMENTO             "Segmento id <%d>, direccion <%p>, size <%zu>\n"


// INFO MENSAJES
#define I__CONEXION_CREATE          "Conexion creada con %s"
#define I__CONEXION_ACCEPT          "Se conecto un cliente"
#define I__DESCONEXION_CLIENTE      "El cliente se desconecto. Terminando servidor"
#define I__SERVER_READY             "Servidor listo para recibir al cliente: "
#define I_ESPERANDO_CONEXION        "Esperando conexiones..."
#define I__CONFIG_GENERIDO_CARGADO  "Config generico creado: %s"

// ERROR MENSAJES
#define E__ARCHIVO_CREATE      "Error al crear/leer archivo %s"
#define E__BAD_REQUEST         "BAD REQUEST"
#define E__CONEXION_CREATE     "Error al crear conexion con %s" // No se esta usando
#define E__CONEXION_CONNECT    "Error al conectar conexion con %s"
#define E__CONEXION_ACEPTAR    "Error al aceptar conexion"
#define E__LOGGER_CREATE       "No se pudo crear logger"
#define E__CONFIG_CREATE       "No se pudo crear config"
#define E__PAQUETE_CREATE      "Error al crear paquete"
#define E__MALLOC_ERROR        "Error al crear el malloc de tamaño %zu "
#define E__CODIGO_INVALIDO     "Error codigo de operacion invalido"

#endif
