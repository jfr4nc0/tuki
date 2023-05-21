/*
 * pcb.h
 *
 *  Created on: Apr 27, 2023
 *      Author: utnso
 */

#ifndef PCB_H_
#define PCB_H_
#include <stdbool.h>
#include <string.h>
#include <commons/collections/list.h>
#include "../../shared/structs.h"

// Cada hilo seria por procesos, no interesa el set de instrucciones que tiene el proceso (PCB)

t_list* pid_list;

typedef struct PCBType{
	int pid; // Identificador del proceso, unico en todo el sistema
	t_list* lista_instrucciones; // Lista de instrucciones a ejecutar
	int program_counter; // Numero de la proxima instruccion a ejecutar
	cpu_registers* cpu_register;
	t_list* lista_segmentos;
	float processor_burst ; // Estimacion utilizada para planificar los procesos en el algoritmo HRRN, la misma tendra un valor inicial definido por archivo de config y sera recalculada bajo la formula de promedio ponderado
	int ready_timestamp; // Timestamp en que el proceso llegó a ready por última vez (utilizado para el cálculo de tiempo de espera del algoritmo HRRN).
	t_list* lista_archivos_abiertos; // Contendrá la lista de archivos abiertos del proceso con la posición del puntero de cada uno de ellos.
}PCB;

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



#endif /* PCB_H_ */
