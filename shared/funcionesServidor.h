#ifndef SERVER_GLOBAL_H_
#define SERVER_GLOBAL_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
// #include <readline/readline.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>

#include "constantes.h"
#include "structs.h"

void* recibir_buffer(int*, int);

int iniciar_servidor(t_config* config, t_log* logger);
int esperar_cliente(int, t_log* logger);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

#endif
