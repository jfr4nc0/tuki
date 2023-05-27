#include "../include/kernel.h"
#include "../../shared/src/funciones.c"
#include "../../shared/src/funcionesCliente.c"
#include "../../shared/src/funcionesServidor.c"

t_log* kernelLogger;
t_kernel_config* kernel_config;

int main(int argc, char** argv) {
	kernelLogger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);
    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, kernelLogger);
    cargar_config_kernel(config, kernelLogger);

    log_debug(kernelLogger, "Vamos a usar el algoritmo %s", kernel_config->ALGORITMO_PLANIFICACION);

    inicializar_semaforos();
    inicializar_listas_estados();
    inicializar_diccionario_recursos();
    inicializar_planificador();

    // Conexiones con los demas modulos
    int conexionCPU = armar_conexion(config, CPU, kernelLogger);
    int conexionMemoria = armar_conexion(config, MEMORIA, kernelLogger);
    int conexionFileSystem = armar_conexion(config, FILE_SYSTEM, kernelLogger);

    int servidorKernel = iniciar_servidor(config, kernelLogger);

    // TODO: Manejar multiples instancias de conexiones de consola al kernel
    inicializar_escucha_conexiones_consolas(servidorKernel);

    return 0;
}

void cargar_config_kernel(t_config* config, t_log* kernelLogger) {
	kernel_config = malloc(sizeof(t_kernel_config));

    kernel_config->IP_MEMORIA = extraer_de_config(config, "IP_MEMORIA", kernelLogger);
    kernel_config->PUERTO_MEMORIA = extraer_de_config(config, "PUERTO_MEMORIA", kernelLogger);
    kernel_config->IP_FILE_SYSTEM = extraer_de_config(config, "IP_FILE_SYSTEM", kernelLogger);
    kernel_config->PUERTO_FILE_SYSTEM = extraer_de_config(config, "PUERTO_FILE_SYSTEM", kernelLogger);
    kernel_config->IP_CPU = extraer_de_config(config, "IP_CPU", kernelLogger);
    kernel_config->PUERTO_CPU = extraer_de_config(config, "PUERTO_CPU", kernelLogger);
    kernel_config->PUERTO_ESCUCHA = extraer_de_config(config, "PUERTO_ESCUCHA", kernelLogger);
    kernel_config->ALGORITMO_PLANIFICACION = extraer_de_config(config, "ALGORITMO_PLANIFICACION", kernelLogger);
    kernel_config->ESTIMACION_INICIAL = extraer_de_config(config, "ESTIMACION_INICIAL", kernelLogger);
    kernel_config->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
    kernel_config->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    kernel_config->RECURSOS = config_get_array_value(config, "RECURSOS");
    kernel_config->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    log_info(kernelLogger, "Config cargada en  'kernel_config' ");

    return;
}

void inicializar_escucha_conexiones_consolas(int servidorKernel){

    log_info(kernelLogger, cantidad_strings_a_mostrar(2), "Esperando conexiones de las consolas...", ENTER);

    while(1){
        int clienteAceptado = esperar_cliente(servidorKernel, kernelLogger);
        log_info(kernelLogger, cantidad_strings_a_mostrar(2), "Consola conectada!", ENTER);
        pthread_t hilo_consola;
        pthread_create(&hilo_consola, NULL, recibir_de_consola, (void*) (intptr_t) clienteAceptado);
        pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
    }
}

void* recibir_de_consola(void *clienteAceptado) {
	int  socketAceptado = (int) (intptr_t)clienteAceptado;
    while(1){  // Queda en un estado de espera activa para la comunicación continua entre los módulos.
        int codigoDeOperacion = recibir_operacion(socketAceptado);
        switch(codigoDeOperacion) {
            case OP_MENSAJE:
                t_list* listaInstrucciones = recibir_paquete(socketAceptado);
                log_info(kernelLogger, cantidad_strings_a_mostrar(2), "Me llegaron los siguientes valores:", ENTER);
                list_iterate(listaInstrucciones, (void*) iterator);
                PCB* pcb = inicializar_pcb(socketAceptado, listaInstrucciones);
                list_destroy(listaInstrucciones);
                break;
        }
    }
}

PCB* inicializar_pcb(int clienteAceptado, t_list* listaInstrucciones) {

	sem_wait(&sem_creacion_pcb);

    PCB* pcb = new_pcb(clienteAceptado, listaInstrucciones);


    log_info(kernelLogger, "valor id: %d", pcb->id_proceso);

    sem_post(&sem_creacion_pcb);

    return pcb;
}

void iterator(char* value) {
    log_info(kernelLogger,"%s", value);
}

// TODO: Cuando se instancia un nuevo PCB, se crea tambien las listas de los elementos necesarios
PCB* new_pcb(int clienteAceptado, t_list* lista_instrucciones) {
//	NUEVO(pcb,PCB);
	PCB* pcb = malloc(sizeof(PCB));
	pcb->id_proceso = contadorProcesoId;
	contadorProcesoId++;
	pcb->lista_instrucciones = lista_instrucciones;

	return pcb;
}
/////////////////////

/* Scheduler */

void inicializar_planificador() {
    log_info(kernelLogger, "Inicialización del planificador FIFO...");
    pthread_create(&planificador_corto_plazo, NULL, (void*) proximo_a_ejecutar, NULL);
    pthread_detach(planificador_corto_plazo);

    // Acá va el manejo de memoria y CPU con hilos.

}

void inicializar_listas_estados() {
	for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
		lista_estados[estado] = list_create();
	}
}

void inicializar_diccionario_recursos() {
    diccionario_recursos = dictionary_create();

    int indice = 0;
    while(kernel_config->RECURSOS[indice] != NULL && kernel_config->INSTANCIAS_RECURSOS[indice] != NULL) {
        crear_cola_recursos(kernel_config->RECURSOS[indice], atoi(kernel_config->INSTANCIAS_RECURSOS[indice]));

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
	sem_init(&sem_grado_multiprogamacion, 0, kernel_config->GRADO_MAX_MULTIPROGRAMACION);
	sem_init(&sem_proceso_en_ready, 0, 0);
	sem_init(&sem_cpu_disponible, 0, 1);
	sem_init(&sem_creacion_pcb, 0, 1);
	// sem_init(&sem_proceso_a_ready,0,1);
}

void proximo_a_ejecutar(){
	while(1){
		sem_wait(&sem_proceso_en_ready);
	    sem_wait(&sem_cpu_disponible);
	    if(strcmp(kernel_config->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
	    	log_info(kernelLogger, "Entre por FIFO");

	        pthread_mutex_lock(&m_lista_READY);
	        PCB* pcb_a_ejecutar = list_remove(lista_estados[ENUM_READY], 0);
	        pthread_mutex_unlock(&m_lista_READY);

	        cambio_de_estado(pcb_a_ejecutar, ENUM_EXECUTING, lista_estados[ENUM_EXECUTING], m_lista_EXECUTING);

	        log_info(kernelLogger, "El proceso %d cambio su estado a RUNNING", pcb_a_ejecutar->id_proceso);
	        log_info(kernelLogger,"PID: %d - Estado Anterior: READY - Estado Actual: RUNNING", pcb_a_ejecutar->id_proceso);

	    } else {
            log_error(kernelLogger, "No es posible utilizar el algoritmo especificado.");
        }
    }
}

void cambio_de_estado(PCB* pcb, pcb_estado estado, t_list* lista, pthread_mutex_t mutex) {
    cambiar_estado_pcb_a(pcb, estado);
    agregar_a_lista(pcb, lista, mutex);
    log_info(kernelLogger, cantidad_strings_a_mostrar(2), "El pcb entro en la cola de ", obtener_nombre_estado(estado));
}

void cambiar_estado_pcb_a(PCB* pcb, pcb_estado nuevoEstado){
    pcb->estado = nuevoEstado;
}

void agregar_a_lista(PCB* pcb, t_list* lista, pthread_mutex_t m_sem){
    pthread_mutex_lock(&m_sem);
    list_add(lista, pcb);
    pthread_mutex_unlock(&m_sem);
}

/////////////
