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


int procesar_instrucciones(int, t_log*);

#endif
