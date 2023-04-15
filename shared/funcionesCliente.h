#ifndef FUNCIONES_CLIENT_GLOBAL_H_
#define FUNCIONES_CLIENT_GLOBAL_H_

// Externas
#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>

// Internas
#include "structs.h"


void paquete(int, t_log*);
int crear_conexion(char*, char*, t_log*);
void enviar_mensaje(char*, int, t_log*);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete*, void*, int);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
int armar_conexion(t_config*, char*, t_log*);

#endif
