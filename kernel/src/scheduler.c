/*
 * scheduler.c
 *
 *  Created on: Apr 29, 2023
 *      Author: utnso
 */
#include "../include/kernel.h"

void inicializar_planificador() {
    log_info(logger, "Inicialización del planificador FIFO...");
    pthread_create(&planificador_corto_plazo, NULL, (void*) proximo_a_ejecutar, NULL);
    pthread_detach(planificador_corto_plazo);

    // Acá va el manejo de memoria y CPU con hilos.

}

void inicializar_listas_estados() {
    lista_NEW = list_create();
	lista_READY = list_create();
	lista_BLOCKED = list_create();
	lista_EXECUTING = list_create();
	lista_EXIT = list_create();
    lista_IO = list_create();
}

void inicializar_diccionario_recursos() {
    diccionario_recursos = dictionary_create();

    int indice = 0;
    while(kernel_config->RECURSOS[indice] != NULL && kernel_config->INSTANCIAS_RECURSOS[indice] != NULL) {
        crear_cola_recursos(kernel_config->RECURSOS[indice], kernel_config->INSTANCIAS_RECURSOS[indice]);

        indice++;
    }
}

void crear_cola_recursos(char* nombre_recurso, int instancias) {

    //t_list* lista_recursos = list_create();
    t_recurso* recurso = malloc(sizeof(t_recurso));

    recurso->nombre = nombre_recurso;
    recurso->instancias = instancias;
    //device->list_processes = io_list;

    sem_t sem;
    sem_init(&sem, 1, 0);
    recurso->sem_recurso = sem;

    dictionary_put(diccionario_recursos, nombre_recurso, recurso);

}

void inicializar_semaforos(){
	sem_init(&sem_grado_multiprogamacion, 0, kernel_config->GRADO_MAX_MULTIPROGRAMACION);
	sem_init(&sem_proceso_en_ready,0, 0);
	sem_init(&sem_cpu_disponible, 0, 1);
	sem_init(&sem_creacion_pcb, 0, 1);
	sem_init(&sem_proceso_a_ready,0,1);

}

void proximo_a_ejecutar(){
	while(1){
		sem_wait(&sem_proceso_en_ready);
	    sem_wait(&sem_cpu_disponible);
	    if(strcmp(kernel_config->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
	    	log_info(logger, "Entre por FIFO");

	        pthread_mutex_lock(&m_lista_READY);
	        t_pcb* pcb_a_ejecutar = list_remove(lista_READY, 0);
	        pthread_mutex_unlock(&m_lista_READY);

	        cambio_de_estado(pcb_a_ejecutar, EXECUTING, lista_EXECUTING, m_lista_EXECUTING);

	        log_info(logger, "El proceso %d cambio su estado a RUNNING", pcb_a_ejecutar->pid);
	        log_info(logger_obligatorio,"PID: %d - Estado Anterior: READY - Estado Actual: RUNNING",pcb_a_ejecutar->pid);

	    } else {
            log_error(logger, "No es posible utilizar el algoritmo especificado.");
        }
    }
}

void cambio_de_estado(t_pcb* pcb, pcb_estado estado, t_list* lista, pthread_mutex_t mutex) {
    cambiar_estado_pcb_a(pcb, estado);
    agregar_a_lista(pcb, lista, mutex);
    log_info(logger, "El pcb entro en la cola de %s", obtener_nombre_estado(estado));
}

void cambiar_estado_pcb_a(t_pcb* pcb, pcb_estado nuevoEstado){
    pcb->estado = nuevoEstado;
}
void agregar_a_lista(t_pcb* pcb, t_list* lista, pthread_mutex_t m_sem){
    pthread_mutex_lock(&m_sem);
    list_add(lista, pcb);
    pthread_mutex_unlock(&m_sem);
}
char* obtener_nombre_estado(pcb_estado estado){
	char* valor_estado;
	switch(estado){
		case NEW :
			valor_estado = string_duplicate("NEW");
			break;
		case READY:
			valor_estado = string_duplicate("READY");
			break;
		case BLOCKED:
			valor_estado = string_duplicate("BLOCKED");
			break;
		case EXECUTING:
			valor_estado = string_duplicate("EXECUTING");
			break;
		case EXIT:
			valor_estado = string_duplicate("EXIT");
			break;
		default:
			valor_estado = string_duplicate("EL ESTADO NO ESTÁ REGISTRADO");
			break;
	}
	return valor_estado;
}
