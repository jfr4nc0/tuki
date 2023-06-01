#include "../include/kernel.h"
#include "../../shared/src/funciones.c"
#include "../../shared/src/funcionesCliente.c"
#include "../../shared/src/funcionesServidor.c"

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

void iterator(char* value) {
    log_info(kernelLogger, "%s ", value);
}

// TODO: Cuando se instancia un nuevo PCB, se crea tambien las listas de los elementos necesarios
PCB* new_pcb(int clienteAceptado, t_list* lista_instrucciones) {
	PCB* pcb = malloc(sizeof(PCB));
	pcb->id_proceso = contadorProcesoId;
	contadorProcesoId++;
    pcb->lista_instrucciones = lista_instrucciones;
    pcb->estado = ENUM_NEW;
    agregar_a_lista_con_sem(pcb, lista_estados[ENUM_NEW], m_lista_estados[ENUM_NEW]);

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
        sem_init(&m_listas[estado], 0, 1);
	}
}

void liberar_listas_estados() {
    for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
        sem_destroy(&m_listas[estado]);
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

void proximo_a_ejecutar() {
	while(1){
		//sem_wait(&sem_proceso_en_ready);
	    //sem_wait(&sem_cpu_disponible);
	    if(strcmp(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
	    	log_info(kernelLogger, "Planificación FIFO escogida.");
	        //PCB* pcbProximo = cambio_de_estado(0, ENUM_READY, ENUM_EXECUTING);
            //sem_post(&sem_proceso_en_ready);

	    	sem_wait(&m_listas[ENUM_READY]);
	    	PCB* pcb = list_remove(lista_estados[ENUM_READY], 0);
	    	sem_post(&m_listas[ENUM_READY]);

	    	cambiar_a(pcb, ENUM_EXECUTING, lista_estados[ENUM_EXECUTING], sem_lista_estados[ENUM_EXECUTING]);
            log_info(logger, "El proceso %d cambio su estado a RUNNING", pcb_to_execute->process_id);
            log_info(mandatory_logger,"PID: %d - Estado Anterior: READY - Estado Actual: RUNNING",pcb_to_execute->process_id);

            send_pcb_package(connection_cpu_dispatch, pcb_to_execute, EXECUTE_PCB);

	    } else if (strcmp(kernelConfig->ALGORITMO_PLANIFICACION, "HRRN")==0) {
	    	// TODO Algoritmo HRRN


        } else {
            log_error(kernelLogger, "No es posible utilizar el algoritmo especificado.");
        }
    }
}



void cambiar_a(PCB* pcb, pcb_estado estado_a_cambiar, t_list* lista_estado_anterior, sem_t* sem_list_estado){
	cambio_de_estado(pcb, estado_a_cambiar);
	agregar_a_lista_con_sem(pcb, lista_estado_anterior, sem_list_estado);
	log_info("El pcb entro en la cola de %d", estado);
}

void cambio_de_estado(PCB* pcb, pcb_estado nuevo_estado){
	pcb->estado = nuevo_estado;
}

void agregar_a_lista_con_sem(PCB* pcb_to_add, t_list* lista, pthread_mutex_t mutex){
	pthread_mutex_lock(&mutex);
	list_add(lista, pcb_to_add);
	pthread_mutex_unlock(&mutex);
}

/*
PCB* cambio_de_estado(int posicion, pcb_estado estadoAnterior, pcb_estado estadoNuevo) {
    // TODO: ¿No puede saberse por el pcb el estado anterior? No se si anda la idea de que devuelva un PCB*
    PCB* pcb = remover_de_lista(posicion, lista_estados[estadoAnterior], m_listas[estadoAnterior]);
    agregar_a_lista(pcb, lista_estados[estadoNuevo], m_listas[estadoNuevo]);
    pcb->estado = estadoNuevo;

    // TODO: ¿No son demasiados logs?
    const char* estadoActual = obtener_nombre_estado(estadoNuevo);
    const char* estadoViejo = obtener_nombre_estado(estadoAnterior);
    log_info(kernelLogger, cantidad_strings_a_mostrar(2), "El pcb entró en la cola de ", estadoActual);
    log_info(kernelLogger, "El proceso %d cambio su estado a %s ", pcb->id_proceso, estadoActual);
    log_info(kernelLogger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->id_proceso, estadoViejo, estadoActual);

    return pcb;
}
*/

void agregar_a_lista(PCB* pcb, t_list* lista, sem_t m_sem) {
    sem_wait(&m_sem);
    list_add(lista, pcb);
    sem_post(&m_sem);
}

PCB* remover_de_lista(int posicion, t_list* lista, sem_t m_sem) {
	sem_wait(&m_sem);
    PCB* pcb = list_remove(lista, posicion);
    sem_post(&m_sem);
    return pcb;
}

/////////////
