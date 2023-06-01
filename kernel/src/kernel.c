#include "../include/kernel.h"
#include "../../shared/src/funciones.c"
#include "../../shared/src/funcionesCliente.c"
#include "../../shared/src/funcionesServidor.c"
#include "../include/scheduler.h"

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

	log_info(kernelLogger, "Inicializando paquete.");
	int  socketAceptado = (int) (intptr_t)clienteAceptado;
    int codigoDeOperacion = recibir_operacion(socketAceptado);
    t_list* listaInstrucciones = recibir_paquete(socketAceptado);

    log_info(kernelLogger, "Me llegaron los siguientes valores: ");
    list_iterate(listaInstrucciones, (void*) iterator);

    PCB* pcb = inicializar_pcb(socketAceptado, listaInstrucciones);
    log_info(kernelLogger, "Se crea el proceso %d en NEW", pcb->id_proceso);
    cambiar_a(pcb, ENUM_NEW, lista_NEW, m_lista_NEW);

    sem_wait(&sem_proceso_a_ready);
    cambiar_a_ready();
    sem_post(&sem_proceso_a_ready);

    list_destroy(listaInstrucciones);

    liberar_conexion(socketAceptado);
}

void cambiar_a_ready(){
	sem_wait(&sem_grado_multiprogamacion);
	PCB* pcb_a_ready;

	// NEW -> READY
	if(!list_is_empty(lista_NEW)) {

	// Primero obtenemos el pcb
	pthread_mutex_lock(&m_lista_NEW);
	pcb_a_ready = list_get(lista_NEW, 0);
	pthread_mutex_unlock(&m_lista_NEW);

		if (string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO") ) {
			cambiar_a(pcb_a_ready, READY, lista_READY, m_lista_READY);

			loggear_cola_ready(kernelConfig->ALGORITMO_PLANIFICACION);

			log_info(kernelLogger, "Entro el pcb #%d a READY!", pcb_a_ready->id_proceso);
			log_info(kernelLogger,"PID: %d - Estado Anterior: NEW - Estado Actual: READY",pcb_a_ready->id_proceso);

			sem_post(&sem_proceso_en_ready);
		}
	}

}

PCB* inicializar_pcb(int clienteAceptado, t_list* listaInstrucciones) {
	int tamanio = 0;
	void* buffer;
	int desplazamiento = 0;

	buffer = recibir_buffer(&tamanio, clienteAceptado);

	//char** listaDeInstrucciones = leer_arreglo_string(buffer, &desplazamiento);
	//char** listaSegmentos = leer_arreglo_string(buffer, &desplazamiento);
	//char** listaArchivosAbiertos = leer_arreglo_string(buffer, &desplazamiento);

	sem_wait(&sem_creacion_pcb);

	PCB* pcb = new_pcb(listaInstrucciones, clienteAceptado, contadorProcesoId);

	contadorProcesoId++;
	log_info(kernelLogger, "valor id: %d", pcb->id_proceso);

	sem_post(&sem_creacion_pcb);

	return pcb;
}

char** leer_arreglo_string(char* buffer, int* desplazamiento){

	int longitud = leer_int(buffer, desplazamiento);

	char** arreglo = malloc((longitud + 1) * sizeof(char*));

	for(int i = 0; i < longitud; i++){
	    arreglo[i] = leer_string(buffer, desplazamiento);
	}
	arreglo[longitud] = NULL;

	return arreglo;
}

void iterator(char* value) {
    log_info(kernelLogger, "%s ", value);
}

// TODO: Cuando se instancia un nuevo PCB, se crea tambien las listas de los elementos necesarios
PCB* new_pcb(char** listaInstrucciones, int clienteAceptado, int contadorProcesoId) {
	PCB* pcb = malloc(sizeof(PCB));

	pcb->id_proceso = contadorProcesoId;
	pcb->estado = ENUM_NEW;
	pcb->lista_instrucciones = listaInstrucciones;
	pcb->program_counter = 0;

	pcb->cpu_register->AX = 0;
	log_info(kernelLogger, "HOLAAAAAAAAA");
	pcb->cpu_register->BX = 0;
	pcb->cpu_register->CX = 0;
	pcb->cpu_register->DX = 0;
	pcb->cpu_register->EAX = 0;
	pcb->cpu_register->EBX = 0;
	pcb->cpu_register->ECX = 0;
	pcb->cpu_register->EDX = 0;
	pcb->cpu_register->RAX = 0;
	pcb->cpu_register->RBX = 0;
	pcb->cpu_register->RCX = 0;
	pcb->cpu_register->RDX = 0;

	t_list* lista_vacia;

	pcb->lista_segmentos = lista_vacia;
	pcb->lista_archivos_abiertos = lista_vacia;
	pcb->processor_burst = kernelConfig->ESTIMACION_INICIAL;
	pcb->ready_timestamp = 0; // TODO: NO SE DE DONDE SE SACA ESTE DATO PARA INICIALIZAR

	/*
	pcb->id_proceso = contadorProcesoId;
	contadorProcesoId++;
    pcb->lista_instrucciones = lista_instrucciones;
    pcb->estado = ENUM_NEW;
    agregar_a_lista(pcb, lista_estados[ENUM_NEW], m_listas[ENUM_NEW]);
	*/
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
		sem_wait(&sem_proceso_en_ready);
	    //sem_wait(&sem_cpu_disponible);
	    if(strcmp(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
	    	log_info(kernelLogger, "Planificación FIFO escogida.");
	        //PCB* pcbProximo = cambio_de_estado(0, ENUM_READY, ENUM_EXECUTING);
            sem_post(&sem_proceso_en_ready);

	    	pthread_mutex_lock(&m_lista_READY);
	    	PCB* pcb = list_remove(lista_READY, 0);
	    	pthread_mutex_unlock(&m_lista_READY);

	    	cambiar_a(pcb, EXECUTING, lista_EXECUTING, m_lista_EXECUTING);
            log_info(kernelLogger, "El proceso %d cambio su estado a RUNNING", pcb->id_proceso);
            log_info(kernelLogger,"PID: %d - Estado Anterior: READY - Estado Actual: RUNNING",pcb->id_proceso);

            envio_pcb(conexionCPU, pcb, OP_EXECUTE_PCB);


	    } else {
            log_error(kernelLogger, "No es posible utilizar el algoritmo especificado.");
        }
    }
}

void envio_pcb(int conexion, PCB* pcb, codigo_operacion codigo){
	t_paquete* paquete = crear_paquete(codigo);
	agregar_pcb_a_paquete(paquete, pcb);
	enviar_paquete(paquete, conexionCPU);
	eliminar_paquete(paquete);

}

void agregar_pcb_a_paquete(t_paquete* paquete, PCB* pcb){
	agregar_int_a_paquete(paquete, pcb->id_proceso);
	agregar_int_a_paquete(paquete, pcb->estado);
	agregar_arreglo_a_paquete(paquete, pcb->lista_instrucciones);
	agregar_int_a_paquete(paquete, pcb->program_counter);
	agregar_registros_a_paquete(paquete, pcb->cpu_register);
	agregar_arreglo_a_paquete(paquete, pcb->lista_segmentos);
	agregar_arreglo_a_paquete(paquete, pcb->lista_archivos_abiertos);
	agregar_float_a_paquete(paquete, pcb->processor_burst);
	agregar_int_a_paquete(paquete, pcb->ready_timestamp);
}

void agregar_int_a_paquete(t_paquete* paquete, int valor){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(int));
	paquete->buffer->size += sizeof(int);
}

void agregar_arreglo_a_paquete(t_paquete* paquete, char** arreglo){
	int tamanio = string_array_size(arreglo);
	agregar_int_a_paquete(paquete, tamanio);
	for(int i = 0; i < tamanio; i++) {
	 	agregar_valor_a_paquete(paquete, arreglo[i], string_length(arreglo[i])+1);
	}
}

void agregar_valor_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void agregar_registros_a_paquete(t_paquete* paquete, registros_cpu* registrosCpu){
	agregar_int_a_paquete(paquete, registrosCpu->AX);
	agregar_int_a_paquete(paquete, registrosCpu->BX);
	agregar_int_a_paquete(paquete, registrosCpu->CX);
	agregar_int_a_paquete(paquete, registrosCpu->DX);
	agregar_long_a_paquete(paquete, registrosCpu->EAX);
	agregar_long_a_paquete(paquete, registrosCpu->EBX);
	agregar_long_a_paquete(paquete, registrosCpu->ECX);
	agregar_long_a_paquete(paquete, registrosCpu->EDX);
	agregar_longlong_a_paquete(paquete, registrosCpu->RAX);
	agregar_longlong_a_paquete(paquete, registrosCpu->RBX);
	agregar_longlong_a_paquete(paquete, registrosCpu->RCX);
	agregar_longlong_a_paquete(paquete, registrosCpu->RDX);


}

void agregar_long_a_paquete(t_paquete* paquete, void* valor){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(long));
	paquete->buffer->size += sizeof(long);
}
void agregar_longlong_a_paquete(t_paquete* paquete, void* valor){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long long));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(long long));
	paquete->buffer->size += sizeof(long long);
}

void agregar_float_a_paquete(t_paquete* paquete, int valor){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(float));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(float));
	paquete->buffer->size += sizeof(float);
}

void cambiar_a(PCB* pcb, pcb_estado estado, t_list* lista, pthread_mutex_t mutex){
	cambio_de_estado(pcb, estado);
	agregar_a_lista_con_sem(pcb, lista, mutex);
	log_info(kernelLogger, "El pcb entro en la cola de %d", (int)estado);
}

void cambio_de_estado(PCB* pcb, pcb_estado nuevo_estado){
	pcb->estado = nuevo_estado;
}

void agregar_a_lista_con_sem(PCB* pcb, t_list* lista, pthread_mutex_t mutex){
	pthread_mutex_lock(&mutex);
	list_add(lista, pcb);
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

void loggear_cola_ready(char* algoritmo){
	char* pids_aux = string_new();
	pids_aux = pids_on_ready();
	log_info(kernelLogger, "Cola Ready %s: %s.", algoritmo, pids_aux);
	free(pids_aux);
}

char* pids_on_ready(){
    char* aux = string_new();
    string_append(&aux,"[");
    int pid_aux;
    for(int i = 0 ; i < list_size(lista_READY); i++){
        PCB* pcb = list_get(lista_READY,i);
        pid_aux = pcb->id_proceso;
        string_append(&aux,string_itoa(pid_aux));
        if(i != list_size(lista_READY)-1) string_append(&aux,"|");
    }
    string_append(&aux,"]");
    return aux;
}

/////////////
