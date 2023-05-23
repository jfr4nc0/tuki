/*
 * pcb.c
 *
 *  Created on: Apr 27, 2023
 *      Author: @jfr4nc0
 */
#include "../include/kernel.h"

// TODO Cuando se instancia un nuevo PCB, se crea tambien las listas de los elementos necesarios
PCB* new_pcb(int clienteAceptado, t_list* lista_instrucciones)
{
	NUEVO(pcb,PCB);
	pcb->id_proceso = contadorProcessId;
	contadorProcessId++;
	pcb->lista_instrucciones = lista_instrucciones;

	return pcb;
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
	return pcb->id_proceso;
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
