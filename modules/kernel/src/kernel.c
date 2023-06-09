#include "kernel.h"

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

    conexionCPU = armar_conexion(config, CPU, kernelLogger);
    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, kernelLogger);
    conexionMemoria = armar_conexion(config, MEMORIA, kernelLogger);
    identificarse(conexionMemoria, AUX_SOY_KERNEL); // Le dice a memoria que módulo se conectó

    int servidorKernel = iniciar_servidor(config, kernelLogger);

    // TODO: Manejar multiples instancias de conexiones de consola al kernel
    inicializar_escucha_conexiones_consolas(servidorKernel);

    /*
    TODO: NUNCA LLEGA ACA PORQUE SE QUEDA ESPERANDO NUEVAS CONSOLAS,
    MOVER ESTAS FUNCIONES A CUANDO EL SISTEMA SOLICITE LA FINALIZACION
    */
    terminar_programa(servidorKernel, kernelLogger, config);
    liberar_recursos_kernel();

    return 0;
}

void cargar_config_kernel(t_config* config, t_log* kernelLogger) {
    kernelConfig = malloc(sizeof(t_kernel_config));

    kernelConfig->IP_MEMORIA = extraer_string_de_config(config, "IP_MEMORIA", kernelLogger);
    kernelConfig->PUERTO_MEMORIA = extraer_string_de_config(config, "PUERTO_MEMORIA", kernelLogger);
    kernelConfig->IP_FILE_SYSTEM = extraer_string_de_config(config, "IP_FILE_SYSTEM", kernelLogger);
    kernelConfig->PUERTO_FILE_SYSTEM = extraer_string_de_config(config, "PUERTO_FILE_SYSTEM", kernelLogger);
    kernelConfig->IP_CPU = extraer_string_de_config(config, "IP_CPU", kernelLogger);
    kernelConfig->PUERTO_CPU = extraer_string_de_config(config, "PUERTO_CPU", kernelLogger);
    kernelConfig->PUERTO_ESCUCHA = extraer_string_de_config(config, "PUERTO_ESCUCHA", kernelLogger);
    kernelConfig->ALGORITMO_PLANIFICACION = extraer_string_de_config(config, "ALGORITMO_PLANIFICACION", kernelLogger);
    kernelConfig->ESTIMACION_INICIAL = extraer_int_de_config(config, "ESTIMACION_INICIAL", kernelLogger);
    kernelConfig->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
    kernelConfig->GRADO_MAX_MULTIPROGRAMACION = extraer_int_de_config(config, "GRADO_MAX_MULTIPROGRAMACION", kernelLogger);
    kernelConfig->RECURSOS = config_get_array_value(config, "RECURSOS");
    kernelConfig->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    log_info(kernelLogger, "Config cargada en  'kernelConfig' ");

    return;
}


void inicializar_escucha_conexiones_consolas(int servidorKernel) {
    while(1) {
        int conexionConConsola = esperar_cliente(servidorKernel, kernelLogger);
        pthread_t hilo_consola;
        pthread_t hilo_procesos_a_ready;
        pthread_create(&hilo_consola, NULL, (void*) recibir_de_consola, (void*) (intptr_t) conexionConConsola);
        pthread_create(&hilo_procesos_a_ready, NULL, (void*) proceso_a_ready, NULL);
		pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
        pthread_detach(hilo_procesos_a_ready);
    }
}

void proceso_a_ready() {
	while(1) {
		sem_wait(&sem_proceso_a_ready);
		sem_wait(&sem_lista_estados[ENUM_NEW]);
		sem_wait(&sem_lista_estados[ENUM_READY]);
		// El tp dice que se obtienen por FIFO
		PCB* pcb = list_get(lista_estados[ENUM_NEW], 0);
		sem_post(&sem_lista_estados[ENUM_NEW]);
		sem_post(&sem_lista_estados[ENUM_READY]);

		// TODO: PONER código donde actualice el PCB andes de moverlo a READY

		cambiar_estado_proceso(pcb, ENUM_READY);
		list_add(lista_estados[ENUM_READY], pcb);

	}
}

void iterator(char* value) {
    log_info(kernelLogger, "%s ", value);
}

void recibir_de_consola(void *clienteAceptado) {

	log_info(kernelLogger, "Inicializando paquete.");
	int  conexionConConsola = (int) (intptr_t)clienteAceptado;
	recibir_operacion(conexionConConsola);
    t_list* listaInstrucciones = recibir_paquete(conexionConConsola);

    log_info(kernelLogger, "Me llegaron los siguientes valores: ");
    list_iterate(listaInstrucciones, (void*) iterator);

	nuevo_proceso(listaInstrucciones, conexionConConsola);

	crear_hilo_planificador();

    list_destroy(listaInstrucciones);

    liberar_conexion(conexionConConsola);

    return;
}

void crear_hilo_planificador() {
    log_info(kernelLogger, "Inicialización del planificador %s...", kernelConfig->ALGORITMO_PLANIFICACION);
    pthread_create(&planificador_corto_plazo, NULL, (void*) proximo_a_ejecutar, NULL);
    pthread_detach(planificador_corto_plazo);

    // Acá va el manejo de memoria y CPU con hilos.

}

void proximo_a_ejecutar() {
	while(1) {
		// Este if tiene sentido? no se puede manejar con semaforos si hay elementos? O conviene un while(1) si no encuentra o algo así?
		if(!list_is_empty(lista_estados[ENUM_READY])) {
			sem_wait(&sem_cpu_disponible);
			PCB* pcbParaEjecutar;

			// TODO: Mover a una función si hay demasiados algoritmos de planificacion distintos
			if(string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "HRRN")) {
				list_sort(lista_estados[ENUM_EXECUTING], (void*) criterio_hrrn);
			}

			sem_wait(&sem_lista_estados[ENUM_READY]);

			sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
			pcbParaEjecutar = list_get(lista_estados[ENUM_READY], 0);
			sem_post(&sem_lista_estados[ENUM_READY]);
			sem_post(&sem_lista_estados[ENUM_EXECUTING]);

			cambiar_estado_proceso(pcbParaEjecutar, ENUM_EXECUTING);
			enviar_operacion(conexionCPU, OP_EXECUTE_PCB, sizeof(PCB), pcbParaEjecutar);
		}
	}
}

/*
 * Ingresa nuevo proceso, por lo tanto se crea un pcb que contiene información del mismo
 *  además se agrega un proceso a lista de new
 *  Devuelve el pcb creado
 *  @param t_list* listaInstrucciones Instrucciones que el proceso debe ejecutar
 *  @param int clienteAceptado cliente que pide crear un proceso nuevo
 *  return PCB*
 */
PCB* nuevo_proceso(t_list* listaInstrucciones, int clienteAceptado) {
	PCB* pcb = malloc(sizeof(PCB));

	pcb->id_proceso = contadorProcesoId;
	contadorProcesoId++;

	pcb->estado = ENUM_NEW;
	pcb->lista_instrucciones = listaInstrucciones;
	pcb->contador_instrucciones = 0;

	pcb->registrosCpu = malloc(sizeof(registros_cpu));
	pcb->registrosCpu->AX = 0;
	pcb->registrosCpu->BX = 0;
	pcb->registrosCpu->CX = 0;
	pcb->registrosCpu->DX = 0;
	pcb->registrosCpu->EAX = 0;
	pcb->registrosCpu->EBX = 0;
	pcb->registrosCpu->ECX = 0;
	pcb->registrosCpu->EDX = 0;
	pcb->registrosCpu->RAX = 0;
	pcb->registrosCpu->RBX = 0;
	pcb->registrosCpu->RCX = 0;
	pcb->registrosCpu->RDX = 0;

	pcb->lista_segmentos = list_create();
	pcb->lista_archivos_abiertos = list_create();
	pcb->processor_burst = kernelConfig->ESTIMACION_INICIAL;
	pcb->ready_timestamp = 0; //TODO
	pcb->hrrn_alfa = kernelConfig->HRRN_ALFA;

	agregar_a_lista_con_sem(pcb, ENUM_NEW);
	log_info(kernelLogger, "Se crea el proceso %d en NEW", pcb->id_proceso);

	sem_post(&sem_proceso_a_ready); // Le envio señal al otro hilo para que cree la estructura y lo mueva a READY cuando pueda

    return pcb;
    //char* list_ids = pids_on_list(ENUM_READY);

	//log_info(kernelLogger, "El pcb entro en la cola de %s", NEW);

    //log_info(kernelLogger, "Cola Ready %s: [%s]",kernelConfig->ALGORITMO_PLANIFICACION,list_ids);
}

/*
 * Esta funcion mueve un proceso de un estado a otro, actualizando listas y pcb
 * @param PCB* pcb PCB que sirve para identificar de que proceso se trata
 * @param pcb_estado estado al que se quiera mover el proceso
 * return void
 */
void cambiar_estado_proceso(PCB* pcb, pcb_estado estadoNuevo) {
	pcb_estado estadoAnterior = pcb->estado;
	pcb->estado = estadoNuevo;
	mover_de_lista_con_sem(pcb, estadoNuevo, estadoAnterior);

	char* estadoAntes = nombres_estados[estadoAnterior];
	char* estadoPosterior = nombres_estados[estadoNuevo];
    log_info(kernelLogger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->id_proceso, estadoAntes, estadoPosterior);
}

void inicializar_listas_estados() {
	for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
		lista_estados[estado] = list_create();
        sem_init(&sem_lista_estados[estado], 0, obtener_recursos(estado));
	}
}

/*
* Devuelve la cantidad de recursos según el estado
* Si el estado es NEW, EXIT O EXECUTING fluctuan entre 0 y 1
*
* Sino, el máximo va a ser el grado maximo de multiprogramacion que es parte de la configuracion dada por config
*/
int obtener_recursos(int estado) {
	return (estado == ENUM_BLOCKED || estado == ENUM_READY) ? kernelConfig->GRADO_MAX_MULTIPROGRAMACION : 1;
}

void liberar_listas_estados() {
    for (int estado = 0; estado < CANTIDAD_ESTADOS; estado++) {
        sem_destroy(&sem_lista_estados[estado]);
	}
}

/*
* Agrega un proceso nuevo a la lista,
* no se me ocurre otro uso que el del estado nuevo
*/
void agregar_a_lista_con_sem(void* elem, int estado) {
	sem_wait(&sem_lista_estados[estado]);
	list_add(lista_estados[estado], elem);
	sem_post(&sem_lista_estados[estado]);
}

void mover_de_lista_con_sem(void* elem, int estadoNuevo, int estadoAnterior) {
	sem_wait(&sem_lista_estados[estadoNuevo]);
	sem_wait(&sem_lista_estados[estadoAnterior]);

	list_remove_element(lista_estados[estadoAnterior], elem);

	sem_post(&sem_lista_estados[estadoAnterior]);
	sem_post(&sem_lista_estados[estadoNuevo]);

	return;
}

char* get_nombre_estado(pcb_estado pcb_estado) {
	if (pcb_estado >= ENUM_NEW) {
		return nombres_estados[pcb_estado];
	}
	return "EL ESTADO NO ESTÁ REGISTRADO"; //TODO: Mejorar este mensaje
}

/*------------ ALGORITMO HRRN -----------------*/
double rafaga_estimada(PCB* pcb) {
	// TODO Usar timestamp.h para tomar el tiempo de ingreso y calcularlo para hrrn
	double alfa = kernelConfig->HRRN_ALFA;
	double ultima_rafaga = pcb->processor_burst;
	double rafaga = ultima_rafaga != 0 ? ((alfa * ultima_rafaga) + ((1 - alfa) * ultima_rafaga)) : kernelConfig->ESTIMACION_INICIAL;

	return rafaga;
}

double calculo_HRRN(PCB* pcb) {
	double rafaga = rafaga_estimada(pcb);
	double res = 1.0 + (pcb->ready_timestamp / rafaga);
	return res;
}

static bool criterio_hrrn(PCB* pcb_A, PCB* pcb_B) {
	double a = calculo_HRRN(pcb_A);
	double b = calculo_HRRN(pcb_B);

	return a <= b;
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

    t_recurso* recurso = malloc(sizeof(t_recurso));

    recurso->nombre = nombre_recurso;
    recurso->instancias = instancias;

    sem_t sem;
    sem_init(&sem, 1, 0);
    recurso->sem_recurso = sem;

    dictionary_put(diccionario_recursos, nombre_recurso, recurso);

}

void inicializar_semaforos() {
	sem_init(&sem_grado_multiprogamacion, 0, kernelConfig->GRADO_MAX_MULTIPROGRAMACION);
	sem_init(&sem_cpu_disponible, 0, 1);
	sem_init(&sem_proceso_a_ready, 0, 0);
}

//////////Funciones para enviar un pcb a cpu //////////////////////////////
void envio_pcb(int conexion, PCB* pcb, codigo_operacion codigo) {
	t_paquete* paquete = crear_paquete(codigo);
	agregar_pcb_a_paquete(paquete, pcb);
	enviar_paquete(paquete, conexionCPU);
	eliminar_paquete(paquete);
}

void agregar_lista_a_paquete(t_paquete* paquete, t_list* lista) {
	int tamanio = list_size(lista);
	agregar_int_a_paquete(paquete, tamanio);

	for(int i = 0; i < tamanio; i++) {
		void* elemento = list_get(lista, i);
		agregar_elemento_a_paquete(paquete, elemento);
		//agregar_valor_a_paquete(paquete, arreglo[i], strlen(arreglo[i]));
	}

}

void agregar_elemento_a_paquete(t_paquete* paquete, void* elemento) {
	agregar_cadena_a_paquete(paquete, (char*)elemento);
}

void agregar_cadena_a_paquete(t_paquete* paquete, char* cadena) {
    int longitud = strlen(cadena) + 1;
    agregar_int_a_paquete(paquete, longitud);
    agregar_valor_a_paquete(paquete, cadena, longitud);
}

void agregar_int_a_paquete(t_paquete* paquete, int valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(int));
    paquete->buffer->size += sizeof(int);
}

void agregar_valor_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);
    paquete->buffer->size += tamanio;
}

void agregar_registros_a_paquete(t_paquete* paquete, registros_cpu* registrosCpu) {
	 agregar_int_a_paquete(paquete, registrosCpu->AX);
	 agregar_int_a_paquete(paquete, registrosCpu->BX);
	 agregar_int_a_paquete(paquete, registrosCpu->CX);
	 agregar_int_a_paquete(paquete, registrosCpu->DX);
	 agregar_long_a_paquete(paquete, &(registrosCpu->EAX));
	 agregar_long_a_paquete(paquete, &(registrosCpu->EBX));
	 agregar_long_a_paquete(paquete, &(registrosCpu->ECX));
	 agregar_long_a_paquete(paquete, &(registrosCpu->EDX));
	 agregar_longlong_a_paquete(paquete, &(registrosCpu->RAX));
	 agregar_longlong_a_paquete(paquete, &(registrosCpu->RBX));
	 agregar_longlong_a_paquete(paquete, &(registrosCpu->RCX));
	 agregar_longlong_a_paquete(paquete, &(registrosCpu->RDX));
}

void agregar_long_a_paquete(t_paquete* paquete, void* valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, sizeof(long));
    paquete->buffer->size += sizeof(long);
}

void agregar_longlong_a_paquete(t_paquete* paquete, void* valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, sizeof(long long));
    paquete->buffer->size += sizeof(long long);
}

void agregar_a_lista(PCB* pcb, t_list* lista, sem_t m_sem) {
    sem_wait(&m_sem);
    list_add(lista, pcb);
    sem_post(&m_sem);
}

void agregar_pcb_a_paquete(t_paquete* paquete, PCB* pcb) {
	agregar_int_a_paquete(paquete, pcb->id_proceso);
	agregar_int_a_paquete(paquete, pcb->estado);
	agregar_lista_a_paquete(paquete, pcb->lista_instrucciones);
	agregar_int_a_paquete(paquete, pcb->contador_instrucciones);
	agregar_registros_a_paquete(paquete, pcb->registrosCpu);
	agregar_lista_a_paquete(paquete, pcb->lista_segmentos);
	agregar_lista_a_paquete(paquete, pcb->lista_archivos_abiertos);
	agregar_int_a_paquete(paquete, pcb->processor_burst);
	agregar_int_a_paquete(paquete, pcb->ready_timestamp);
}
////////////////////////////////////////

char* pids_on_list(pcb_estado estado) {
    char* aux = string_new();
    string_append(&aux,"[");
    int pid_aux;
    for(int i = 0 ; i < list_size(lista_estados[estado]); i++) {
    	PCB* pcb = list_get(lista_estados[estado],i);
        pid_aux = pcb->id_proceso;
        string_append(&aux,string_itoa(pid_aux));
        if(i != list_size(lista_estados[estado])-1) string_append(&aux,"|");
    }
    string_append(&aux,"]");
    return aux;
}

void loggear_cola_lista(pcb_estado estado) {
	char* pids_aux = string_new();
    char* algoritmo = kernelConfig->ALGORITMO_PLANIFICACION;
	pids_aux = pids_on_list(estado);
	char* estado2 = nombres_estados[estado];
	log_info(kernelLogger, "Cola %s %s: %s.",estado2, algoritmo, pids_aux);
	free(pids_aux);
}
