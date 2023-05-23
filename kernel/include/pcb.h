/*
 * pcb.h
 *
 *  Created on: Apr 27, 2023
 *      Author: utnso
 */

#ifndef PCB_H_
#define PCB_H_


#include <stddef.h>
#include <stdlib.h>
#include <commons/collections/list.h>

// Elimina la inclusión de "scheduler.h"

#include "../../shared/structs.h"
#include "constantes.h"
#include "../../shared/constantes.h"
#include "../../shared/constructor.h"
#include "../../shared/funciones.h"
#include "../../shared/funcionesCliente.h"
#include "../../shared/funcionesServidor.h"
// Cada hilo seria por procesos, no interesa el set de instrucciones que tiene el proceso (PCB)

t_list* pid_list;

int contadorProcesoId = 0;

typedef struct {
	int ID;
	// direccion_base, de que tipo??
	int tamanio;
}t_segmento;

typedef struct {
    int ID; // File descriptor
    int posicion_puntero; // Posición del puntero
} archivo_abierto_t;

/*************************** Getters && Setters of PCB ***************************/

PCB* new_pcb(int, t_list*);
int set_pid();
int get_pid();
int set_program_counter();
int get_program_counter();
t_list* set_lista_instrucciones();
t_list* get_lista_instrucciones();
cpu_registers* set_registro_cpu();
cpu_registers* get_registro_cpu();
t_list* set_lista_segmentos();
t_list* get_lista_segmentos();
float set_processor_burst();
float get_processor_burst();
int set_ready_timestamp();
int get_ready_timestamp();
t_list* set_lista_archivos_abiertos();
t_list* get_lista_archivos_abiertos();

#endif
