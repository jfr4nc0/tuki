#include "../include/kernel.h"
#include "../../shared/src/funciones.c"

int main(int argc, char** argv) {
    logger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);

    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, logger);
    cargar_config(config);
    log_warning(logger, "Vamos a usar el algoritmo %s", kernel_config->ALGORITMO_PLANIFICACION);

    inicializar_listas_estados();
    inicializar_diccionario_recursos();

    inicializar_semaforos();

    // Conexiones con los demas modulos
    conexionCPU = armar_conexion(config, CPU, logger);
    conexionMemoria = armar_conexion(config, MEMORIA, logger);
    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, logger);

    int servidorKernel = iniciar_servidor(config, logger);

    inicializar_planificador();

    return 0;
}

void cargar_config(t_config* config){
	kernel_config = malloc(sizeof(t_kernel_config));
	kernel_config->IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
	kernel_config->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	kernel_config->PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	kernel_config->IP_FILE_SYSTEM = config_get_string_value(config, "IP_FILE_SYSTEM");
	kernel_config->PUERTO_FILE_SYSTEM = config_get_string_value(config, "PUERTO_FILE_SYSTEM");
	kernel_config->IP_CPU = config_get_string_value(config, "IP_CPU");
	kernel_config->PUERTO_CPU = config_get_string_value(config, "PUERTO_CPU");
	kernel_config->PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	kernel_config->ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	kernel_config->ESTIMACION_INICIAL = config_get_string_value(config, "ESTIMACION_INICIAL");
	kernel_config->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
	kernel_config->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
	kernel_config->RECURSOS = config_get_array_value(config, "RECURSOS");
	kernel_config->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");

	log_info(logger, "Config cargada en  'kernel_config' ");
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
	// sem_init(&sem_io, 0, 0);
	sem_init(&sem_cpu_disponible, 0, 1);
	sem_init(&sem_creacion_pcb, 0, 1);
	// sem_init(&sem_inicializar_memoria,0,0);
	sem_init(&sem_proceso_a_ready,0,1);
}



void inicializar_planificador() {
    log_info(logger, "Inicializando hilos...");
    pthread_create(&planificador_corto_plazo, NULL, (void*) proximo_a_ejecutar, NULL);
    pthread_detach(planificador_corto_plazo);

    /*
    pthread_create(&thread_memory, NULL, (void*) manage_memory, NULL);
    pthread_detach(thread_memory);

    pthread_create(&thread_cpu, NULL, (void*) manage_cpu, (void*) conexionCPU);
    pthread_detach(thread_cpu);
	*/
}

void proximo_a_ejecutar(){
	while(1){
		sem_wait(&sem_proceso_en_ready);
	    sem_wait(&sem_cpu_disponible);

	    if(strcmp(kernel_config->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
	    	log_info(logger, "Entre por FIFO");

	        pthread_mutex_lock(&m_lista_READY);
	        //PCB* pcb_a_ejecutar = list_remove(lista_READY, 0);
	        pthread_mutex_unlock(&m_lista_READY);

	        //cambio_de_estado(pcb_a_ejecutar, EXECUTING, lista_EXECUTING, m_lista_EXECUTING);

	        //log_info(logger, "El proceso %d cambio su estado a RUNNING", pcb_a_ejecutar->pid);
	        //log_info(logger_obligatorio,"PID: %d - Estado Anterior: READY - Estado Actual: RUNNING",pcb_a_ejecutar->pid);

	    } else {
            log_error(logger, "No es posible utilizar el algoritmo especificado.");
        }
    }

}
/*
void cambio_de_estado(t_pcb* pcb, pcb_status state, t_list* list, pthread_mutex_t mutex) {
    change_pcb_state_to(pcb, state);
    add_to_list_with_sems(pcb, list, mutex);
    log_info(logger, "El pcb entro en la cola de %s", get_state_name(state));
}
*/
/*
void manage_memory(){
}
void manage_cpu(){
}
*/






