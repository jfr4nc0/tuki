#ifndef SERVER_GLOBAL_H_
#define SERVER_GLOBAL_H_

#include "structs.h"
#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include "constantes.h"
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>

void* recibir_buffer(int*, int);

int iniciar_servidor(t_config* config, char* modulo);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

extern t_log* logger;

#endif
