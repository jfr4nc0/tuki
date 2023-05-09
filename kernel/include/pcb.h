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

// Cada hilo seria por procesos, no interesa el set de instrucciones que tiene el proceso (PCB)

typedef struct CPURegister{

	uint32_t AX;
	uint32_t BX;
	uint32_t CX;
	uint32_t DX;

	uint64_t EAX;
	uint64_t EBX;
	uint64_t ECX;
	uint64_t EDX;

	__int128_t RAX;
	__int128_t RBX;
	__int128_t RCX;
	__int128_t RDX;

}cpu_register_t;

t_list* pid_list;

typedef struct PCBType{
	int pid; // Identificador del proceso, unico en todo el sistema
	t_list* lista_instrucciones; // Lista de instrucciones a ejecutar
	int program_counter; // Numero de la proxima instruccion a ejecutar
	cpu_register_t* cpu_register;
	t_list* lista_segmentos;
	float processor_burst ; // Estimacion utilizada para planificar los procesos en el algoritmo HRRN, la misma tendra un valor inicial definido por archivo de config y sera recalculada bajo la formula de promedio ponderado
	int ready_timestamp; // Timestamp en que el proceso llegó a ready por última vez (utilizado para el cálculo de tiempo de espera del algoritmo HRRN).
	t_list* lista_archivos_abiertos; // Contendrá la lista de archivos abiertos del proceso con la posición del puntero de cada uno de ellos.
}PCB;

/*************************** Getters && Setters of PCB ***************************/

int set_pid();
int get_pid();
int set_program_counter();
int get_program_counter();
t_list* set_lista_instrucciones();
t_list* get_lista_instrucciones();
cpu_register_t* set_registro_cpu();
cpu_register_t* get_registro_cpu();
t_list* set_lista_segmentos();
t_list* get_lista_segmentos();
float set_processor_burst();
float get_processor_burst();
int set_ready_timestamp();
int get_ready_timestamp();
t_list* set_lista_archivos_abiertos();
t_list* get_lista_archivos_abiertos();



#endif /* PCB_H_ */
