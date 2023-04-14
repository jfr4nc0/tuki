#ifndef CONSTANTES_GLOBALES_H_
#define CONSTANTES_GLOBALES_H_

// TODO separar configuraciones en archivo más especifico
/////////////////////////////////////////////////////////

// Configuracion General
#define LONGITUD_MAXIMA_CADENA          1000
#define PATH_DEFAULT_CONEXION_KERNEL    "../kernel_conexion.config"

// TODO: Borrar
#define IP "127.0.0.1"
#define PUERTO "4444"

/* CONFIGURACIONES INDIVIDUALES POR MODULO*/

#define LOG_LEVEL_CONSOLA                           LOG_LEVEL_INFO
#define LOG_LEVEL_CPU                               LOG_LEVEL_INFO
#define LOG_LEVEL_FILE_SYSTEM                       LOG_LEVEL_INFO
#define LOG_LEVEL_KERNEL                            LOG_LEVEL_TRACE
#define LOG_LEVEL_MEMORIA                           LOG_LEVEL_INFO

#define MOSTRAR_OCULTAR_MENSAJES_LOG_CONSOLA        1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_CPU            1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_FILE_SYSTEM    1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_MEMORIA        1
#define MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL         1

////////////////////////////////////////////////////////

// Signos
#define ENTER             "\n"
#define SIGN_CONSOLA      "> "
#define EMPTY_STRING      ""

// CONSTANTES
#define MODO_LECTURA_ARCHIVO 	"r"
#define MODULO_CONSOLA			"CONSOLA"
#define MODULO_CPU              "CPU"
#define MODULO_MEMORIA          "MEMORIA"
#define MODULO_FILE_SYSTEM		"FILE_SYSTEM"
#define MODULO_KERNEL			"KERNEL"

#define IP_CONFIG 				"IP_"
#define PUERTO_CONFIG 			"PUERTO_"

// DEBUG MENSAJES
#define D__ESTABLECIENDO_CONEXION   "Estableciendo conexion"
#define D__CONFIG_CREADO            "Config creado"
#define D__LOG_CREADO               "Log creado"

// INFO MENSAJES
#define I__CONEXION_CREATE          "Conexion creada"
#define I__DESCONEXION_CLIENTE      "El cliente se desconecto. Terminando servidor"
#define I__SERVER_READY             "Servidor listo para recibir al cliente: "


// ERROR MENSAJES
#define E__ARCHIVO_CREATE   "Error al crear/leer archivo"
#define E__BAD_REQUEST 	    "BAD REQUEST"
#define E__CONEXION_CREATE  "Error al crear conexion"
#define E__CONEXION_CONNECT "Error al conectar conexion"
#define E__LOGGER_CREATE    "No se pudo crear logger"
#define E__CONFIG_CREATE    "No se pudo crear config"
#define E__PAQUETE_CREATE   "Error al crear paquete"

#endif
