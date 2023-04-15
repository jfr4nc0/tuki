#ifndef CONFIGURACIONES_GLOBALES_H_
#define CONFIGURACIONES_GLOBALES_H_

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
#define LOG_LEVEL_DEFAULT 		LOG_LEVEL_INFO

#endif
