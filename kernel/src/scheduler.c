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

void inicializar_listas_estados(t_list* lista_estados[CANTIDAD_ESTADOS]) {
	for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
		lista_estados[estado] = list_create();
	}
}

void inicializar_diccionario_recursos(t_kernel_config* kernel_config) {
    diccionario_recursos = dictionary_create();

    int indice = 0;
    while(kernel_config->RECURSOS[indice] != NULL && kernel_config->INSTANCIAS_RECURSOS[indice] != NULL) {
        crear_cola_recursos(kernel_config->RECURSOS[indice], kernel_config->INSTANCIAS_RECURSOS[indice]);

        indice++;
    }
}

void crear_cola_recursos(char* nombre_recurso, int instancias) {

    // t_list* lista_recursos = list_create();
    t_recurso* recurso = malloc(sizeof(t_recurso));

    recurso->nombre = nombre_recurso;
    recurso->instancias = instancias;
    // device->list_processes = io_list;

    sem_t sem;
    sem_init(&sem, 1, 0);
    recurso->sem_recurso = sem;

    dictionary_put(diccionario_recursos, nombre_recurso, recurso);

}

void inicializar_semaforos() {
	sem_init(&sem_grado_multiprogamacion, 0, kernel_config.GRADO_MAX_MULTIPROGRAMACION);
	sem_init(&sem_proceso_en_ready, 0, 0);
	sem_init(&sem_cpu_disponible, 0, 1);
	sem_init(&sem_creacion_pcb, 0, 1);
	// sem_init(&sem_proceso_a_ready,0,1);
}

void proximo_a_ejecutar(){
	while(1){
		sem_wait(&sem_proceso_en_ready);
	    sem_wait(&sem_cpu_disponible);
	    if(strcmp(kernel_config.ALGORITMO_PLANIFICACION, "FIFO") == 0) {
	    	log_info(logger, "Entre por FIFO");

	        pthread_mutex_lock(&m_lista_READY);
	        PCB* pcb_a_ejecutar = list_remove(lista_estados[ENUM_READY], 0);
	        pthread_mutex_unlock(&m_lista_READY);

	        cambio_de_estado(pcb_a_ejecutar, EXECUTING, lista_estados[ENUM_EXECUTING], m_lista_EXECUTING);

	        log_info(logger, "El proceso %d cambio su estado a RUNNING", pcb_a_ejecutar->id_proceso);
	        log_info(logger,"PID: %d - Estado Anterior: READY - Estado Actual: RUNNING", pcb_a_ejecutar->id_proceso);

	    } else {
            log_error(logger, "No es posible utilizar el algoritmo especificado.");
        }
    }
}

void cambio_de_estado(PCB* pcb, pcb_estado estado, t_list* lista, pthread_mutex_t mutex) {
    cambiar_estado_pcb_a(pcb, estado);
    agregar_a_lista(pcb, lista, mutex);
    log_info(logger, cantidad_strings_a_mostrar(2), "El pcb entro en la cola de ", obtener_nombre_estado(estado));
}

void cambiar_estado_pcb_a(PCB* pcb, pcb_estado nuevoEstado){
    pcb->estado = nuevoEstado;
}

void agregar_a_lista(PCB* pcb, t_list* lista, pthread_mutex_t m_sem){
    pthread_mutex_lock(&m_sem);
    list_add(lista, pcb);
    pthread_mutex_unlock(&m_sem);
}

char* obtener_nombre_estado(pcb_estado estado){
	if (estado >= ENUM_NEW) {
		return string_duplicate(nombres_estados[estado]);
	}
	return string_duplicate("EL ESTADO NO ESTÁ REGISTRADO");
}
