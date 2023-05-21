/*
 * pcb.c
 *
 *  Created on: Apr 27, 2023
 *      Author: @jfr4nc0
 */

#include <stdlib.h>
#include "../include/pcb.h"
#include "../../shared/constructor.h"

// TODO Cuando se instancia un nuevo PCB, se crea tambien las listas de los elementos necesarios
PCB* new_PCB(int pid, t_list lista_instrucciones,int program_counter, cpu_registers cpu_register, t_list lista_segmentos,float processor_burst, int ready_timestamp, t_list lista_archivos_abiertos)
{
	NUEVO(new_pcb,PCB);
	new_pcb->pid = set_pid();
	new_pcb->lista_instrucciones = set_lista_instrucciones(lista_instrucciones);
	new_pcb->program_counter = set_program_counter();
	new_pcb->cpu_register = set_registro_cpu();
	new_pcb->lista_segmentos = set_lista_segmentos();
	new_pcb->processor_burst = set_processor_burst();
	new_pcb->ready_timestamp = set_ready_timestamp();
	new_pcb->lista_archivos_abiertos = set_lista_archivos_abiertos();

	return new_pcb;
}


/*************************** Getters && Setters of PCB **************************

// Funcion que genere un pid unico en el sistema
int set_pid()
{
	int new_pid,size;

	if(list_is_empty(pid_list)){
		new_pid = 1;
		pid_list = list_create();
		list_add(pid_list,new_pid);
	}

	size = list_size(pid_list);
	new_pid = list_get(pid_list,size); // list_get(pid_list,size--) value of last element

	list_add(pid_list,new_pid++);

	return new_pid;
}

int get_pid(PCB* pcb){
	return pcb->pid;
}

int set_program_counter(){
	int program_counter;

	return program_counter;
}
int get_program_counter(PCB* pcb){
	return pcb->program_counter;
}

// TODO set_lista_instrucciones
t_list* set_lista_instrucciones(t_list* lista_instrucciones){
	return lista_instrucciones;
}

t_list* get_lista_instrucciones(PCB* pcb){
	return pcb->lista_instrucciones;
}

// TODO set_registro_cpu
cpu_registers* set_registro_cpu(){
	cpu_registers* registro_cpu;

	return registro_cpu;
}

cpu_registers* get_registro_cpu(PCB* pcb){
	return pcb->cpu_register;
}

// TODO set_lista_segmentos
t_list* set_lista_segmentos(){
	t_list* lista_segmentos = list_create();

	return lista_segmentos;
}

t_list* get_lista_segmentos(PCB* pcb){
	return pcb->lista_segmentos;
}

// TODO set_processor_burst
float set_processor_burst(){
	float processor_burst;

	return processor_burst;
}

float get_processor_burst(PCB* pcb){
	return pcb->processor_burst;
}

// TODO set_ready_timestamp
int set_ready_timestamp(){
	int ready_timestamp;

	return ready_timestamp;
}

int get_ready_timestamp(PCB* pcb){
	return pcb->ready_timestamp;
}

// TODO set_lista_archivos_abiertos
t_list* set_lista_archivos_abiertos(){
	t_list* lista_archivos_abiertos;

	return lista_archivos_abiertos;
}

t_list* get_lista_archivos_abiertos(PCB* pcb){
	return pcb->lista_archivos_abiertos;
}

*/