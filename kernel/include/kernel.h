#ifndef KERNEL_H_
#define KERNEL_H_

// Librerias externas
#include "includesExternas.h"

// Librerias Internas

#include "../../shared/structs.h"
#include "constantes.h"
#include "../../shared/constantes.h"
#include "../../shared/constructor.h"
#include "../../shared/funciones.h"
#include "../../shared/funcionesCliente.h"
#include "../../shared/funcionesServidor.h"
#include "pcb.h"
#include "scheduler.h"


int conexionCPU;
int conexionMemoria;
int conexionFileSystem;
int servidorKernel;

#include "variablesGlobales.h"

t_list* procesar_instrucciones(int, t_list*, t_log*, t_config*);
t_kernel_config inicializar_config(char*, t_log*);
t_kernel_config cargar_config_kernel(t_config*, t_log*);
void inicializar_escucha_conexiones_consolas(int);
void* recibir_de_consola(void*);
void iterator(char* value);

PCB* inicializar_pcb(int, t_list*);


#endif
