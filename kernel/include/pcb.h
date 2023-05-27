/*
 * pcb.h
 *
 *  Created on: Apr 27, 2023
 *      Author: utnso
 */

#ifndef PCB_H_
#define PCB_H_
/*
#include <stddef.h>
#include <stdlib.h>
#include <commons/collections/list.h>

t_list* pid_list;

int contadorProcesoId = 0;

typedef struct {
	int  id;
	// direccion_base, de que tipo??
	int tamanio;
}t_segmento;

typedef struct {
    int  id; // File descriptor
    int posicion_puntero;
} archivo_abierto_t;

PCB* new_pcb(int, t_list*);

/*************************** Getters && Setters of PCB ***************************/

/*
int set_pid();
int get_pid();
int set_contador_instrucciones();
int get_contador_instrucciones();
t_list* set_lista_instrucciones();
t_list* get_lista_instrucciones();
registros_cpu* set_registro_cpu();
registros_cpu* get_registro_cpu();
t_list* set_lista_segmentos();
t_list* get_lista_segmentos();
float set_processor_burst();
float get_processor_burst();
int set_ready_timestamp();
int get_ready_timestamp();
t_list* set_lista_archivos_abiertos();
t_list* get_lista_archivos_abiertos();
*/

#endif
