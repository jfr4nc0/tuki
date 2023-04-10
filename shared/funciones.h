#include "./utils.h"
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>

t_log* iniciar_logger(char*);
t_config* iniciar_config(char*);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
int armar_conexion(t_config*, t_log*);
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
int validarArgumentos(int argc, char** argv);
