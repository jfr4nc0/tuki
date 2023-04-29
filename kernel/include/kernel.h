#ifndef KERNEL_H_
#define KERNEL_H_

// Externas
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include <commons/log.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<assert.h>

// Internas
#include "utils.h"
#include "funciones.h"
#include "constantes.h"
#include "../../shared/constantes.h"
#include "../../shared/structs.h"

extern t_log* logger;

pthread_t planificador_corto_plazo;
pthread_t thread_memory;
pthread_t thread_dispatch;

void inicializar_planificador();

void schedule_next_to_running();
void manage_memory();
void manage_dispatch();




#endif
