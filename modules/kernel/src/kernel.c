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

    conexionCPU = armar_conexion(config, CPU, kernelLogger);
    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, kernelLogger);
    conexionMemoria = armar_conexion(config, MEMORIA, kernelLogger);
    identificarse(conexionMemoria, AUX_SOY_KERNEL); // Le dice a memoria que módulo se conectó

    inicializar_semaforos();
    inicializar_listas_estados();
    inicializar_diccionario_recursos();
    inicializar_planificador();

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
        pthread_create(&hilo_consola, NULL, recibir_de_consola, (void*) (intptr_t) conexionConConsola);
        pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
    }
}

void iterator(char* value) {
    log_info(kernelLogger, "%s ", value);
}

void* recibir_de_consola(void *clienteAceptado) {

	log_info(kernelLogger, "Inicializando paquete.");
	int  conexionConConsola = (int) (intptr_t)clienteAceptado;
	recibir_operacion(conexionConConsola);
    t_list* listaInstrucciones = recibir_paquete(conexionConConsola);

    log_info(kernelLogger, "Me llegaron los siguientes valores: ");
    list_iterate(listaInstrucciones, (void*) iterator);

	PCB* pcb = new_pcb(listaInstrucciones, conexionConConsola);

    cambiar_a_ready();

	proximo_a_ejecutar();

    list_destroy(listaInstrucciones);

    liberar_conexion(conexionConConsola);

    return NULL;

}

void inicializar_planificador() {
    log_info(kernelLogger, "Inicialización del planificador %s...", kernelConfig->ALGORITMO_PLANIFICACION);
    pthread_create(&planificador_corto_plazo, NULL, (void*) proximo_a_ejecutar, NULL);
    pthread_detach(planificador_corto_plazo);

    // Acá va el manejo de memoria y CPU con hilos.

}

void cambiar_a_ready() {

	sem_wait(&sem_grado_multiprogamacion);
	PCB* pcb_a_ready;

	// NEW a READY
	if(!list_is_empty(lista_estados[ENUM_NEW])) {

		sem_wait(&sem_lista_estados[ENUM_NEW]);
		pcb_a_ready = list_get(lista_estados[ENUM_NEW], 0);
		sem_post(&sem_lista_estados[ENUM_NEW]);

		if (string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO") ) {
			cambiar_estado_pcb(pcb_a_ready, ENUM_READY, 0);

			loggear_cola_lista(ENUM_READY);

			sem_post(&sem_proceso_en_ready);
		} else if(string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "HRRN")) {
//			TODO: Evaluar ingreso a READY
//			Primero el de mayor tasa de respuesta. (Basado en prioridades)
//			Tasa de Respuesta -> R = W + S / S
//			Siendo W tiempo de espera y S tiempo de ráfaga esperado.
//			Cuanto mayor R, mayor prioridad
//			Tiene en cuenta la edad del proceso (por W, tiempo de espera). Por lo tanto elimina el problema de inanición

			cambiar_estado_pcb(pcb_a_ready, ENUM_READY, 0);

			loggear_cola_lista(ENUM_READY);

			sem_post(&sem_proceso_en_ready);
		}
	}

}


PCB* new_pcb(t_list* listaInstrucciones, int clienteAceptado) {
	PCB* pcb = malloc(sizeof(PCB));

	sem_wait(&sem_creacion_pcb);
	pcb->id_proceso = contadorProcesoId;
	contadorProcesoId++;
	sem_post(&sem_creacion_pcb);

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
	pcb->ready_timestamp = 0;
	pcb->hrrn_alfa = kernelConfig->HRRN_ALFA;

    char* list_ids = pids_on_list(ENUM_READY);
	agregar_a_lista_con_sem(pcb, lista_estados[ENUM_NEW], sem_lista_estados[ENUM_NEW]);

    log_info(kernelLogger, "Cola Ready %s: [%s]",kernelConfig->ALGORITMO_PLANIFICACION,list_ids);

return pcb;

}

/*
 * Esta funcion toma el ultimo elemento de la lista del estado anterior (pcb_estado) y
 * lo agrega a la cola de la lista del estado posterior, y cambia el estado del pcb
 * Ejemplo: cambiar_estado_pcb(0,ENUM_READY,ENUM_EXECUTING)
 */
void cambiar_estado_pcb(PCB* pcb, pcb_estado estado_nuevo, int posicion) {
	pcb_estado estado_anterior = pcb->estado;
	pcb->estado = estado_nuevo;
	agregar_a_lista_con_sem(pcb, lista_estados[estado_nuevo], sem_lista_estados[estado_nuevo]);

	char* estadoAnterior = get_nombre_estado(estado_anterior);
	char* estadoPosterior = get_nombre_estado(estado_nuevo);
    log_info(kernelLogger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->id_proceso, estadoAnterior, estadoPosterior);
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

void agregar_a_lista_con_sem(void* elem, t_list* lista, sem_t sem_lista) {
	sem_wait(&sem_lista);
	list_add(lista,elem);
	sem_post(&sem_lista);
}

char* get_nombre_estado(pcb_estado pcb_estado) {
	if (pcb_estado >= ENUM_NEW) {
		return nombres_estados[pcb_estado];
	}
	return "EL ESTADO NO ESTÁ REGISTRADO"; //TODO: Mejorar este mensaje
}

/*------------ ALGORITMO FIFO -----------------*/

void planificar_FIFO(int cpu_conexion) {
	pcb_estado estado = ENUM_READY;
	sem_wait(&sem_lista_estados[estado]);
	PCB* pcb = list_remove(lista_estados[estado],0);
	sem_post(&sem_lista_estados[estado]);

	cambiar_estado_pcb(pcb,ENUM_EXECUTING,0);

	envio_pcb(cpu_conexion, pcb, OP_EXECUTE_PCB);
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

void planificar_HRRN(int cpu_conexion) {
	pcb_estado estado = ENUM_READY;
	// Recorrer la lista de pcb y calcular HRRN
	sem_wait(&sem_lista_estados[estado]);
	list_sort(lista_estados[estado], (void*) criterio_hrrn);
	PCB* pcb = list_remove(lista_estados[estado],0);
	sem_post(&sem_lista_estados[estado]);

	cambiar_estado_pcb(pcb,ENUM_EXECUTING,0);

	envio_pcb(cpu_conexion, pcb, OP_EXECUTE_PCB);
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
	sem_init(&sem_proceso_en_ready, 0, 0);
	sem_init(&sem_cpu_disponible, 0, 1);
	sem_init(&sem_creacion_pcb, 0, 1);
	sem_init(&sem_proceso_a_ready,0,1);

	for (int i = 0; i < CANTIDAD_ESTADOS; i++) {
		sem_init(&sem_lista_estados[i], 0, 1);
	}
}

void proximo_a_ejecutar() {
	while(1) {
		sem_wait(&sem_proceso_en_ready);
	    sem_wait(&sem_cpu_disponible);
	    if(strcmp(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO") == 0) {
	    	log_info(kernelLogger, "Planificación FIFO escogida.");

	    	planificar_FIFO(conexionCPU);

	    } else if (strcmp(kernelConfig->ALGORITMO_PLANIFICACION, "HRRN")==0) {
	    	log_info(kernelLogger, "Planificación HRRN escogida.");

	    	planificar_HRRN(conexionCPU);

	    } else {
            log_error(kernelLogger, "No es posible utilizar el algoritmo especificado.");
        }
    }
}

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

PCB* remover_de_lista(int posicion, t_list* lista, sem_t m_sem) {
	sem_wait(&m_sem);
    PCB* pcb = list_remove(lista, posicion);
    sem_post(&m_sem);
    return pcb;
}

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
	char* estado2 = get_nombre_estado(estado);
	log_info(kernelLogger, "Cola %s %s: %s.",estado2, algoritmo, pids_aux);
	free(pids_aux);
}
