#include "planificador.h"

void inicializar_listas_estados() {
	for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
		lista_estados[estado] = list_create();
        sem_init(&sem_lista_estados[estado], 0, 1);
	}

}

void liberar_listas_estados() {
    for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
        sem_destroy(&sem_lista_estados[estado]);
	}
}

void agregar_a_lista_con_sem(void* elem, t_list* lista, sem_t sem_lista){
	sem_wait(&sem_lista);
	list_add(lista,elem);
	sem_post(&sem_lista);
}

char* get_nombre_estado(pcb_estado pcb_estado){
	if (pcb_estado >= ENUM_NEW) {
		return nombres_estados[pcb_estado];
	}
	return "EL ESTADO NO ESTÁ REGISTRADO"; //TODO: Mejorar este mensaje
}

/*------------ ALGORITMO FIFO -----------------*/

void planificar_FIFO(){
	pcb_estado estado = ENUM_READY;
	sem_wait(&sem_lista_estados[estado]);
	PCB* pcb = list_remove(lista_estados[estado],0);
	sem_post(&sem_lista_estados[estado]);

	cambiar_estado_pcb(pcb,ENUM_EXECUTING,0);

	envio_pcb(conexionCPU, pcb, OP_EXECUTE_PCB);
}

/*------------ ALGORITMO HRRN -----------------*/
double rafaga_estimada(PCB* pcb){
	double alfa = kernelConfig->HRRN_ALFA;
	double ultima_rafaga = pcb->processor_burst;
	double rafaga = ultima_rafaga != 0 ? ((alfa * ultima_rafaga) + ((1 - alfa) * ultima_rafaga)) : kernelConfig->ESTIMACION_INICIAL;

	return rafaga;
}

double calculo_HRRN(PCB* pcb){
	double rafaga = rafaga_estimada(pcb);
	double res = 1.0 + (pcb->ready_timestamp / rafaga);
	return res;
}

static bool criterio_hrrn(PCB* pcb_A, PCB* pcb_B){
	double a = calculo_HRRN(pcb_A);
	double b = calculo_HRRN(pcb_B);

	return a <= b;
}

void planificar_HRRN(){
	pcb_estado estado = ENUM_READY;
	// Recorrer la lista de pcb y calcular HRRN
	sem_wait(&sem_lista_estados[estado]);
	list_sort(lista_estados[estado], (void*) criterio_hrrn);
	PCB* pcb = list_remove(lista_estados[estado],0);
	sem_post(&sem_lista_estados[estado]);

	cambiar_estado_pcb(pcb,ENUM_EXECUTING,0);

	envio_pcb(conexionCPU, pcb, OP_EXECUTE_PCB);
}


