#include "kernel.h"

t_log* kernelLogger;
t_kernel_config* kernelConfig;

void liberar_recursos_kernel() {
    free(kernelConfig);
    liberar_listas_estados();
    liberar_conexion(conexionCPU);
    liberar_conexion(conexionMemoria);
    liberar_conexion(conexionFileSystem);
}


int main(int argc, char** argv) {
	kernelLogger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);
    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, kernelLogger);
    cargar_config_kernel(config, kernelLogger);

    log_debug(kernelLogger, "Vamos a usar el algoritmo %s", kernelConfig->ALGORITMO_PLANIFICACION);

    inicializar_semaforos();
    inicializar_listas_estados();
    inicializar_diccionario_recursos();
    inicializar_planificador();

    // Conexiones con los demas modulos
    conexionCPU = armar_conexion(config, CPU, kernelLogger);
    conexionMemoria = armar_conexion(config, MEMORIA, kernelLogger);
    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, kernelLogger);

    int servidorKernel = iniciar_servidor(config, kernelLogger);

    // TODO: Manejar multiples instancias de conexiones de consola al kernel
    inicializar_escucha_conexiones_consolas(servidorKernel);

    terminar_programa(servidorKernel, kernelLogger, config);
    liberar_recursos_kernel();

    return 0;
}

void cargar_config_kernel(t_config* config, t_log* kernelLogger) {
    kernelConfig = malloc(sizeof(t_kernel_config));

    kernelConfig->IP_MEMORIA = extraer_de_config(config, "IP_MEMORIA", kernelLogger);
    kernelConfig->PUERTO_MEMORIA = extraer_de_config(config, "PUERTO_MEMORIA", kernelLogger);
    kernelConfig->IP_FILE_SYSTEM = extraer_de_config(config, "IP_FILE_SYSTEM", kernelLogger);
    kernelConfig->PUERTO_FILE_SYSTEM = extraer_de_config(config, "PUERTO_FILE_SYSTEM", kernelLogger);
    kernelConfig->IP_CPU = extraer_de_config(config, "IP_CPU", kernelLogger);
    kernelConfig->PUERTO_CPU = extraer_de_config(config, "PUERTO_CPU", kernelLogger);
    kernelConfig->PUERTO_ESCUCHA = extraer_de_config(config, "PUERTO_ESCUCHA", kernelLogger);
    kernelConfig->ALGORITMO_PLANIFICACION = extraer_de_config(config, "ALGORITMO_PLANIFICACION", kernelLogger);
    kernelConfig->ESTIMACION_INICIAL = extraer_de_config(config, "ESTIMACION_INICIAL", kernelLogger);
    kernelConfig->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
    kernelConfig->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    kernelConfig->RECURSOS = config_get_array_value(config, "RECURSOS");
    kernelConfig->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    log_info(kernelLogger, "Config cargada en  'kernelConfig' ");

    return;
}

void inicializar_escucha_conexiones_consolas(int servidorKernel){
    while(1){
        int clienteAceptado = esperar_cliente(servidorKernel, kernelLogger);
        pthread_t hilo_consola;
        pthread_create(&hilo_consola, NULL, recibir_de_consola, (void*) (intptr_t) clienteAceptado);
        pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
    }
}

void* recibir_de_consola(void *clienteAceptado) {
	int  socketAceptado = (int) (intptr_t)clienteAceptado;
    int codigoDeOperacion = recibir_operacion(socketAceptado);

    switch(codigoDeOperacion) {
        case OP_PAQUETE:
            t_list* listaInstrucciones = recibir_paquete(socketAceptado);

            log_info(kernelLogger, "Me llegaron los siguientes valores: ");
            list_iterate(listaInstrucciones, (void*) iterator);

            PCB* pcb = inicializar_pcb(socketAceptado, listaInstrucciones);

            proximo_a_ejecutar();

            list_destroy(listaInstrucciones);
            break;
    }

    liberar_conexion(socketAceptado);
}

PCB* inicializar_pcb(int clienteAceptado, t_list* listaInstrucciones) {
	sem_wait(&sem_creacion_pcb);
    PCB* pcb = new_pcb(clienteAceptado, listaInstrucciones);
    log_info(kernelLogger, "valor id: %d", pcb->id_proceso);
    sem_post(&sem_creacion_pcb);

    return pcb;
}

char* ids_from_list_to_string(t_list* lista){
	char* ids = string_new();
	int pid_aux;
	for(int i=0;i<list_size(&lista);i++){
		PCB* pcb = list_get(lista,i);
		pid_aux = pcb->id_proceso;
		string_append(&ids, string_itoa(pid_aux));
		if(i!=list_size(&lista)-1) string_append(&ids,",");
	}

	return ids;
}

// TODO: Cuando se instancia un nuevo PCB, se crea tambien las listas de los elementos necesarios
PCB* new_pcb(int clienteAceptado, t_list* lista_instrucciones) {
	PCB* pcb = malloc(sizeof(PCB));
	pcb->id_proceso = contadorProcesoId;
	contadorProcesoId++;
    pcb->lista_instrucciones = lista_instrucciones;
    pcb->estado = ENUM_NEW;

    agregar_a_lista_con_sem(pcb, lista_estados[ENUM_NEW], sem_lista_estados[ENUM_NEW]);

    char* list_ids = ids_from_list_to_string(lista_estados[ENUM_READY]);

//    log_info(kernelLogger, "Cola Ready %s: [%s]"s,kernelConfig->ALGORITMO_PLANIFICACION,list_ids);

	return pcb;
}


void iterator(char* value) {
    log_info(kernelLogger, "%s ", value);
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
        sem_init(&sem_lista_estados[estado], 0, 1);
	}
}

void liberar_listas_estados() {
    for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
        sem_destroy(&sem_lista_estados[estado]);
	}
}

void inicializar_diccionario_recursos() {
    diccionario_recursos = dictionary_create();

    int indice = 0;
    while(kernelConfig->RECURSOS[indice] != NULL && kernelConfig->INSTANCIAS_RECURSOS[indice] != NULL) {
        crear_cola_recursos(kernelConfig->RECURSOS[indice], atoi(kernelConfig->INSTANCIAS_RECURSOS[indice]));

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
	sem_init(&sem_grado_multiprogamacion, 0, kernelConfig->GRADO_MAX_MULTIPROGRAMACION);
	sem_init(&sem_proceso_en_ready, 0, 0);
	sem_init(&sem_cpu_disponible, 0, 1);
	sem_init(&sem_creacion_pcb, 0, 1);
	sem_init(&sem_proceso_a_ready,0,1);
}


void agregar_a_lista_con_sem(void* elem, t_list* lista, sem_t sem_lista){
	sem_wait(&sem_lista);
	list_add(lista,elem);
	sem_post(&sem_lista);
}


/*
 * Esta funcion toma el ultimo elemento de la lista del estado anterior (pcb_estado) y
 * lo agrega a la cola de la lista del estado posterior, y cambia el estado del pcb
 * Ejemplo: cambiar_estado_pcb(0,ENUM_READY,ENUM_EXECUTING)
 */
void cambiar_estado_pcb(int posicion, pcb_estado estado_anterior, pcb_estado estado_posterior){
	sem_wait(&sem_lista_estados[estado_anterior]);
	PCB* pcb = list_remove(lista_estados[estado_anterior],posicion);
	sem_post(&sem_lista_estados[estado_posterior]);

	pcb->estado = estado_posterior;

	agregar_a_lista_con_sem(pcb, lista_estados[estado_posterior], sem_lista_estados[estado_posterior]);

	char* estadoAnterior = obtener_nombre_estado(estado_anterior);
	char* estadoPosterior = obtener_nombre_estado(estado_posterior);
    log_info(kernelLogger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->id_proceso, estadoAnterior, estadoPosterior);
}


void proximo_a_ejecutar() {
	while(1){
//		sem_wait(&sem_proceso_en_ready);
	    sem_wait(&sem_cpu_disponible);
	    if(strcmp(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
	    	log_info(kernelLogger, "Planificación FIFO escogida.");

	    	cambiar_estado_pcb(0,ENUM_READY,ENUM_EXECUTING);

//            send_pcb_package(connection_cpu_dispatch, pcb_to_execute, EXECUTE_PCB);

	    } else if (strcmp(kernelConfig->ALGORITMO_PLANIFICACION, "HRRN")==0) {
	    	// TODO Algoritmo HRRN


        } else {
            log_error(kernelLogger, "No es posible utilizar el algoritmo especificado.");
        }
    }
}

/////////////
