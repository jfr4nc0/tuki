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

    conexionMemoria = armar_conexion(config, MEMORIA, kernelLogger);
    conexionCPU = armar_conexion(config, CPU, kernelLogger);
    log_info(kernelLogger, "Conexion cpu inicial: %d", conexionCPU);
    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, kernelLogger);

    enviar_codigo_operacion(conexionMemoria, AUX_SOY_KERNEL); // Le dice a memoria que módulo se conectó

    inicializar_estructuras();

    inicializar_diccionario_recursos();

    int servidorKernel = iniciar_servidor(config, kernelLogger);


    // TODO: Manejar multiples instancias de conexiones de consola al kernel
    inicializar_escucha_conexiones_consolas(servidorKernel);

    /*TODO: NUNCA LLEGA ACA PORQUE SE QUEDA ESPERANDO NUEVAS CONSOLAS,
    MOVER ESTAS FUNCIONES A CUANDO EL SISTEMA SOLICITE LA FINALIZACION*/

    terminar_programa(servidorKernel, kernelLogger, config);
    liberar_recursos_kernel();

    return 0;
}

void inicializar_estructuras(){

	inicializar_listas_estados();

	inicializar_semaforos();
	crear_hilo_planificadores();

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
        pthread_create(&hilo_consola, NULL, (void*) recibir_de_consola, (void*) (intptr_t) conexionConConsola);
        pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
    }
}

void enviar_proceso_a_ready() {
	while(1) {
		sem_wait(&sem_proceso_a_ready_inicializar);
		sem_wait(&sem_lista_estados[ENUM_NEW]);
		// El tp dice que se obtienen por FIFO
		PCB* pcb = list_get(lista_estados[ENUM_NEW], 0);

		if (conexionMemoria > 0) {
			// Creo el pcb en memoria
			enviar_operacion(conexionMemoria, AUX_CREATE_PCB, sizeof(int), &pcb->id_proceso);
			codigo_operacion codigoRespuesta = recibir_operacion(conexionMemoria);
			if (codigoRespuesta == AUX_ERROR) {
				log_error(kernelLogger, "Segmentation fault la creacion del proceso %d, ", pcb->id_proceso);
			} else if (codigoRespuesta == AUX_SOLO_CON_COMPACTACION) {
				log_info(kernelLogger, "Se necesita compactar para proceso %d", pcb->id_proceso);
			} else if (codigoRespuesta == AUX_OK) {
				t_list* segmentos = recibir_paquete(conexionMemoria);
				log_debug(kernelLogger, "Se creó en memoria el proceso %d, semgmentos creados: %d", pcb->id_proceso, list_size(segmentos));
				pcb->lista_segmentos = segmentos;
			} else {
				log_error(kernelLogger, "Error interno en Modulo Memoria para crear proceso id: %d.", pcb->id_proceso);
			}
		}

		sem_wait(&sem_lista_estados[ENUM_READY]);
		cambiar_estado_proceso_sin_semaforos(pcb, ENUM_READY);
		sem_post(&sem_lista_estados[ENUM_NEW]);

		list_add(lista_estados[ENUM_READY], pcb);
		sem_post(&sem_lista_estados[ENUM_READY]);

		sem_post(&sem_proceso_a_ready_terminado);
	}
}

void iterator(char* value) {
    log_info(kernelLogger, "%s ", value);
}
//TODO: REVISAR, NO LLEGUE
void recibir_de_consola(void *clienteAceptado) {

	log_info(kernelLogger, "Inicializando paquete.");
	int  conexionConConsola = (int) (intptr_t)clienteAceptado;
	recibir_operacion(conexionConConsola);
    t_list* listaInstrucciones = recibir_paquete(conexionConConsola);

    log_info(kernelLogger, "Me llegaron los siguientes valores: ");
    list_iterate(listaInstrucciones, (void*) iterator);

	nuevo_proceso(listaInstrucciones, conexionConConsola);

	//crear_hilo_planificador(); //TODO: ESTO ESTA MAL ACA

    list_destroy(listaInstrucciones);

    //liberar_conexion(conexionConConsola);

    return;
}

void crear_hilo_planificadores() {
    log_info(kernelLogger, "Inicialización del planificador %s...", kernelConfig->ALGORITMO_PLANIFICACION);

    _planificador_largo_plazo();

    pthread_create(&planificador_corto_plazo, NULL, (void*) _planificador_corto_plazo, NULL);
    pthread_detach(planificador_corto_plazo);
}

void _planificador_largo_plazo() {
	// TODO: AGREGAR SEMAFOROS ACA, VOS PODES SOFY Y FRAN
    // pthread_t m_liberar_pcb_de_exit;
    // pthread_create(&m_liberar_pcb_de_exit, NULL, (void*) liberar_pcb_de_exit, NULL);
    // pthread_detach(m_liberar_pcb_de_exit);

	pthread_t hilo_procesos_a_ready;
    pthread_create(&hilo_procesos_a_ready, NULL, (void*) enviar_proceso_a_ready, NULL);
    pthread_detach(hilo_procesos_a_ready);

    return;
}

void* liberar_pcb_de_exit(void* args){
    for (;;) {
        PCB *pcb_a_liberar = desencolar_primer_pcb(ENUM_EXIT);

        // TODO: MEMORIA --> adapter_memoria_finalizar_proceso(pcbALiberar);
        //stream_send_empty_buffer(pcb_get_socket(pcbALiberar), HEADER_proceso_terminado);

        //destruir_pcb(pcb_a_liberar);
        sem_post(&sem_grado_multiprogamacion);
    }

    return NULL;
}
/*
void destruir_pcb(PCB* pcb) {
    //pthread_mutex_lock(pcb_get_mutex(pcb));

    t_buffer *instrucciones = pcb->lista_instrucciones;
    if (instrucciones != NULL) {
        free(instrucciones->stream);
        free(instrucciones);
    }

    registros_cpu *registrosCpu = pcb->registrosCpu;
    if (registrosCpu != NULL) {
    	if (registrosCpu->AX != NULL) free(registrosCpu->AX);
    	if (registrosCpu->BX != NULL) free(registrosCpu->BX);
    	if (registrosCpu->CX != NULL) free(registrosCpu->CX);
    	if (registrosCpu->DX != NULL) free(registrosCpu->DX);
    	// Registros 8 bytes
    	if (registrosCpu->EAX != NULL) free(registrosCpu->EAX);
    	if (registrosCpu->EBX != NULL) free(registrosCpu->EBX);
    	if (registrosCpu->ECX != NULL) free(registrosCpu->ECX);
    	if (registrosCpu->EDX != NULL) free(registrosCpu->EDX);
    	// Registros 16 bytes
    	if (registrosCpu->RAX != NULL) free(registrosCpu->RAX);
    	if (registrosCpu->RBX != NULL) free(registrosCpu->RBX);
    	if (registrosCpu->RCX != NULL) free(registrosCpu->RCX);
    	if (registrosCpu->RDX != NULL) free(registrosCpu->RDX);

    	free(registrosCpu);
    }

    list_destroy(pcb->lista_segmentos);

    list_destroy(pcb->lista_archivos_abiertos);

    free(pcb);
}
*/
char* get_nombre_estado(pcb_estado pcb_estado) {
	if (pcb_estado >= ENUM_NEW) {
		return nombres_estados[pcb_estado];
	}
	return "EL ESTADO NO ESTÁ REGISTRADO"; //TODO: Mejorar este mensaje
}

PCB* desencolar_primer_pcb(pcb_estado estado) {
    int estadoNumerico = (int)estado;
	sem_wait(&sem_lista_estados[estadoNumerico]);
    // pthread_mutex_lock(mutex_lista_estados[estado_]);
    PCB *pcb = list_remove(lista_estados[estadoNumerico], 0);
    sem_post(&sem_lista_estados[estadoNumerico]);
    // pthread_mutex_unlock(mutex_lista_estados[estado_]);

    return pcb;
}


void _planificador_corto_plazo() {

	// Desalojo de PCBs
	pthread_t manejo_desalojo;
	// pthread_create(&manejo_desalojo, NULL, manejo_desalojo_pcb, NULL); //TODO
	// pthread_detach(manejo_desalojo);

	//Dispatcher
	while(1) {
		sem_wait(&sem_cpu_disponible);
		sem_wait(&sem_proceso_a_ready_terminado);
		PCB* pcbParaEjecutar;

		if(string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "FIFO")) {
			pcbParaEjecutar = elegir_pcb_segun_fifo();
		}
		else if (string_equals_ignore_case(kernelConfig->ALGORITMO_PLANIFICACION, "HRRN")) {
			pcbParaEjecutar = elegir_pcb_segun_hrrn();
		}

		cambiar_estado_proceso(pcbParaEjecutar, ENUM_EXECUTING);

		log_trace(kernelLogger, "---------------MOSTRANDO PCB A ENVIAR A CPU---------------");
		mostrar_pcb(pcbParaEjecutar);

		sem_post(&sem_proceso_a_executing);
	}
}

void *manejo_desalojo_pcb() {

	// int clienteKernel = (int) (intptr_t)socket;

	for(;;){
		sem_wait(&sem_proceso_a_executing);
		sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
		PCB* pcb_en_ejecucion = list_get(lista_estados[ENUM_EXECUTING], 0);
		sem_post(&sem_lista_estados[ENUM_EXECUTING]);

		timestamp inicio_ejecucion_proceso;
		timestamp fin_ejecucion_proceso;

		set_timespec(&inicio_ejecucion_proceso);
		envio_pcb_a_cpu(conexionCPU, pcb_en_ejecucion, OP_EXECUTE_PCB);
		codigo_operacion codigo = recibir_proceso_desajolado(pcb_en_ejecucion, conexionCPU);
		set_timespec(&fin_ejecucion_proceso);

		 // Actualizo el estimado en el pcb segun el real ejecutado
		 double tiempo_en_cpu = obtener_diferencial_de_tiempo_en_milisegundos(&fin_ejecucion_proceso, &inicio_ejecucion_proceso);
		 pcb_estimar_proxima_rafaga(pcb_en_ejecucion, tiempo_en_cpu);

		 switch(codigo){
		 	 case DESALOJO_YIELD:
		 	 {
		 		 log_error(kernelLogger, "SI FUNCIONA ME MUERO");
		 	 }
		 }


	}
	return NULL;
}

double obtener_diferencial_de_tiempo_en_milisegundos(timestamp *end, timestamp *start) {
    const uint32_t SECS_TO_MILISECS = 1000;
    const uint32_t NANOSECS_TO_MILISECS = 1000000;
    return (double) ( (end->tv_sec - start->tv_sec) * SECS_TO_MILISECS + (end->tv_nsec - start->tv_nsec) / NANOSECS_TO_MILISECS );
}
void pcb_estimar_proxima_rafaga(PCB *pcb_ejecutado, double tiempo_en_cpu){
    double alfa_hrrn = kernelConfig->HRRN_ALFA;

    double estimadoProxRafagaPcb = pcb_ejecutado->estimacion_rafaga;
    double estimadoProxRafagaActualizado = alfa_hrrn * tiempo_en_cpu + (1.0 - alfa_hrrn) * estimadoProxRafagaPcb;
    pcb_ejecutado->estimacion_rafaga = estimadoProxRafagaActualizado;
    return;
}
codigo_operacion recibir_proceso_desajolado(PCB* pcb_en_ejecucion, int socket_cpu) {

	PCB* pcb_recibido;
	codigo_operacion codigo;

	pcb_recibido = recibir_pcb_de_cpu(socket_cpu, &codigo);

	int id_proceso_en_ejecucion = pcb_en_ejecucion->id_proceso;
	int id_pcb_recibido = pcb_recibido->id_proceso;

	if(id_proceso_en_ejecucion != id_pcb_recibido) {
	    log_error(kernelLogger, "El PID: %d del proceso desalojado no coincide con el proceso en ejecución con PID: %d", id_proceso_en_ejecucion, id_pcb_recibido);
	    exit(EXIT_FAILURE);
	}


	return codigo;
}

PCB* recibir_pcb_de_cpu(int clienteAceptado, codigo_operacion* codigo) {
	PCB* pcb = malloc(sizeof(PCB));

	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

	codigo_operacion codigoOperacion = recibir_operacion(clienteAceptado);
	log_info(kernelLogger, "CODIGO DE OPERACION RECIBIDO: %d", codigoOperacion);
	*codigo = codigoOperacion;
	buffer = recibir_buffer(&tamanio, clienteAceptado);

	pcb->id_proceso = leer_int(buffer, &desplazamiento);

	pcb->estado = leer_int(buffer, &desplazamiento);

	pcb->lista_instrucciones = leer_string_array(buffer, &desplazamiento); // NO esta funcionando bien

	pcb->contador_instrucciones = leer_int(buffer, &desplazamiento);

	pcb->lista_segmentos = leer_string_array(buffer, &desplazamiento); //TODO: Modificar cuando se mergee memoria

	pcb->lista_archivos_abiertos = list_create();
	int cantidad_de_archivos = leer_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_archivos; i++) {
			archivo_abierto_t* archivo_abierto = malloc(sizeof(archivo_abierto_t));

		    archivo_abierto->id = leer_int(buffer, &desplazamiento);
		    archivo_abierto->posicion_puntero = leer_int(buffer, &desplazamiento);

		    list_add(pcb->lista_archivos_abiertos, archivo_abierto);
		    free(archivo_abierto);
	}

	pcb->registrosCpu = malloc(sizeof(registros_cpu));
	strcpy(pcb->registrosCpu->AX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->BX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->CX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->DX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EAX, leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EBX, leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->ECX, leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EDX, leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RAX, leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RBX, leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RCX, leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RDX, leer_registro_16_bytes(buffer, &desplazamiento));

	pcb->estimacion_rafaga = leer_double(buffer, &desplazamiento);
	pcb->ready_timestamp = leer_double(buffer, &desplazamiento);
	log_info(kernelLogger, "---------------------------------------------Recibi el proceso de PID: %d", pcb->id_proceso);
	mostrar_pcb(pcb);
	return pcb;
}


/*
PCB* recibir_proceso_desalojado(int socket){
	PCB* pcb = malloc(sizeof(PCB));

	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

	buffer = recibir_buffer(&tamanio, socket);

	pcb->id_proceso = leer_int(buffer, &desplazamiento);

	pcb->estado = leer_int(buffer, &desplazamiento);

	pcb->lista_instrucciones = leer_string_array(buffer, &desplazamiento); // NO esta funcionando bien

	pcb->contador_instrucciones = leer_int(buffer, &desplazamiento);

	pcb->lista_segmentos = leer_string_array(buffer, &desplazamiento); //TODO: Modificar cuando se mergee memoria

	pcb->lista_archivos_abiertos = list_create();
	int cantidad_de_archivos = leer_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_archivos; i++) {
	archivo_abierto_t* archivo_abierto = malloc(sizeof(archivo_abierto_t));

	archivo_abierto->id = leer_int(buffer, &desplazamiento);
	archivo_abierto->posicion_puntero = leer_int(buffer, &desplazamiento);

	list_add(pcb->lista_archivos_abiertos, archivo_abierto);
	free(archivo_abierto);
	}

	pcb->registrosCpu = malloc(sizeof(registros_cpu));
	pcb->registrosCpu->AX = leer_registro_4_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->BX = leer_registro_4_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->CX = leer_registro_4_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->DX = leer_registro_4_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->EAX = leer_registro_8_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->EBX = leer_registro_8_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->ECX = leer_registro_8_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->EDX = leer_registro_8_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->RAX = leer_registro_16_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->RBX = leer_registro_16_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->RCX = leer_registro_16_bytes(buffer, &desplazamiento);
	pcb->registrosCpu->RDX = leer_registro_16_bytes(buffer, &desplazamiento);

	pcb->estimacion_rafaga = leer_double(buffer, &desplazamiento);
	pcb->ready_timestamp = leer_double(buffer, &desplazamiento);

	return pcb;
}
*/
void set_timespec(timestamp *timespec)
{
    int retVal = clock_gettime(CLOCK_REALTIME, timespec);

    if (retVal == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
}

PCB* elegir_pcb_segun_fifo(){
	PCB* pcb;
	sem_wait(&sem_lista_estados[ENUM_READY]);
	//sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
	pcb = list_get(lista_estados[ENUM_READY], 0);
	sem_post(&sem_lista_estados[ENUM_READY]);
	//sem_post(&sem_lista_estados[ENUM_EXECUTING]);

	return pcb;

}

PCB* elegir_pcb_segun_hrrn(){
	PCB* pcb;
	sem_wait(&sem_lista_estados[ENUM_READY]);
	//sem_wait(&sem_lista_estados[ENUM_EXECUTING]);
	list_sort(lista_estados[ENUM_READY], (void*) criterio_hrrn);
	pcb = list_get(lista_estados[ENUM_READY], 0);
	sem_post(&sem_lista_estados[ENUM_READY]);
	//sem_post(&sem_lista_estados[ENUM_EXECUTING]);

	return pcb;
}

void mostrar_pcb(PCB* pcb){
	log_trace(kernelLogger, "PID: %d", pcb->id_proceso);
	char* estado = nombres_estados[pcb->estado];
	log_trace(kernelLogger, "ESTADO: %s", estado);
	log_trace(kernelLogger, "INSTRUCCIONES A EJECUTAR: ");
	list_iterate(pcb->lista_instrucciones, (void*) iterator);
	log_trace(kernelLogger, "PROGRAM COUNTER: %d", pcb->contador_instrucciones);
	log_trace(kernelLogger, "Registro AX: %s", pcb->registrosCpu->AX);
	log_trace(kernelLogger, "Registro BX: %s", pcb->registrosCpu->BX);
	log_trace(kernelLogger, "Registro CX: %s", pcb->registrosCpu->CX);
	log_trace(kernelLogger, "Registro DX: %s", pcb->registrosCpu->DX);
	log_trace(kernelLogger, "Registro EAX: %s", pcb->registrosCpu->EAX);
	log_trace(kernelLogger, "Registro EBX: %s", pcb->registrosCpu->EBX);
	log_trace(kernelLogger, "Registro ECX: %s", pcb->registrosCpu->ECX);
	log_trace(kernelLogger, "Registro EDX: %s", pcb->registrosCpu->EDX);
	log_trace(kernelLogger, "Registro RAX: %s", pcb->registrosCpu->RAX);
	log_trace(kernelLogger, "Registro RBX: %s", pcb->registrosCpu->RBX);
	log_trace(kernelLogger, "Registro RCX: %s", pcb->registrosCpu->RCX);
	log_trace(kernelLogger, "Registro RDX: %s", pcb->registrosCpu->RDX);
	log_trace(kernelLogger, "LISTA SEGMENTOS: ");
	list_iterate(pcb->lista_segmentos, (void*) iterator);
	log_trace(kernelLogger, "LISTA ARCHIVOS ABIERTOS: ");
	list_iterate(pcb->lista_archivos_abiertos, (void*) iterator);
	log_trace(kernelLogger, "ESTIMACION HHRN: %f", pcb->estimacion_rafaga);
	log_trace(kernelLogger, "TIMESTAMP EN EL QUE EL PROCESO LLEGO A READY POR ULTIMA VEZ: %f", pcb->ready_timestamp);
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
	pcb->lista_instrucciones = list_create();
	list_add_all(pcb->lista_instrucciones, listaInstrucciones);
	pcb->contador_instrucciones = 0;

	pcb->registrosCpu = malloc(sizeof(registros_cpu));
	strcpy(pcb->registrosCpu->AX, "_AX");
	strcpy(pcb->registrosCpu->BX, "_BX");
	strcpy(pcb->registrosCpu->CX, "_CX");
	strcpy(pcb->registrosCpu->DX, "_DX");
	strcpy(pcb->registrosCpu->EAX, "R_EAX");
	strcpy(pcb->registrosCpu->EBX, "R_EBX");
	strcpy(pcb->registrosCpu->ECX, "R_EcX");
	strcpy(pcb->registrosCpu->EDX, "R_EDX");
	strcpy(pcb->registrosCpu->RAX, "R_RAX");
	strcpy(pcb->registrosCpu->RBX, "R_RBX");
	strcpy(pcb->registrosCpu->RCX, "R_RCX");
	strcpy(pcb->registrosCpu->RDX, "R_RDX");

	pcb->lista_segmentos = list_create();
	pcb->lista_archivos_abiertos = list_create();
	pcb->estimacion_rafaga = kernelConfig->ESTIMACION_INICIAL;
	pcb->ready_timestamp = 0; //TODO
	//pcb->hrrn_alfa = kernelConfig->HRRN_ALFA;

	agregar_a_lista_con_sem(pcb, ENUM_NEW);
	log_info(kernelLogger, "Se crea el proceso %d en NEW", pcb->id_proceso);

	sem_post(&sem_proceso_a_ready_inicializar); // Le envio señal al otro hilo para que cree la estructura y lo mueva a READY cuando pueda

    return pcb;
    //char* list_ids = pids_on_list(ENUM_READY);

	//log_info(kernelLogger, "El pcb entro en la cola de %s", NEW);

    //log_info(kernelLogger, "Cola Ready %s: [%s]",kernelConfig->ALGORITMO_PLANIFICACION,list_ids);
}


void cambiar_estado_proceso_sin_semaforos(PCB* pcb, pcb_estado estadoNuevo) {
	pcb_estado estadoAnterior = pcb->estado;
	pcb->estado = estadoNuevo;
	list_remove_element(lista_estados[estadoAnterior], pcb);
	list_add(lista_estados[estadoNuevo], pcb);

	char* estadoAntes = nombres_estados[estadoAnterior];
	char* estadoPosterior = nombres_estados[estadoNuevo];
    log_info(kernelLogger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->id_proceso, estadoAntes, estadoPosterior);
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

        //sem_init(&sem_lista_estados[estado], 0, );
		sem_init(&sem_lista_estados[estado], 0, 1);
		// mutex_lista_estados[estado] = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		// pthread_mutex_init(mutex_lista_estados[estado], NULL);

	}
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
	list_add(lista_estados[estadoNuevo], elem);

	sem_post(&sem_lista_estados[estadoAnterior]);
	sem_post(&sem_lista_estados[estadoNuevo]);

	return;
}

/*------------ ALGORITMO HRRN -----------------*/
double rafaga_estimada(PCB* pcb) {
	// TODO Usar timestamp.h para tomar el tiempo de ingreso y calcularlo para hrrn
	double alfa = kernelConfig->HRRN_ALFA;
	double ultima_rafaga = pcb->estimacion_rafaga;
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
	sem_init(&sem_proceso_a_ready_inicializar, 0, 0);
	sem_init(&sem_proceso_a_ready_terminado, 0, 0);
	sem_init(&sem_proceso_a_executing, 0, 0);
	/* TODO: JOAN Y JOACO
	 * LOS CHICOS TENIAN ESTOS TAMBIEN Y ES PROBABLE QUE LOS NECESITEN
	 *
	    pthread_mutex_init(&mutexSocketMemoria, NULL);
    	pthread_mutex_init(&mutexSocketFilesystem, NULL);
    	sem_init(&semFRead, 0, 1);
    	sem_init(&semFWrite, 0, 1);
    	fRead = false;
    	fWrite = false;
	 */
}

//////////Funciones para enviar un pcb a cpu //////////////////////////////
void envio_pcb(int conexion, PCB* pcb, codigo_operacion codigo) {
	t_paquete* paquete = crear_paquete(codigo);
	agregar_pcb_a_paquete(paquete, pcb);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}
// ---------------------------------------PRUEBAS
void envio_pcb_a_cpu(int conexion, PCB* pcb_a_enviar, codigo_operacion codigo) {
	t_paquete* paquete = crear_paquete(codigo);
	agregar_pcb_a_paquete_para_cpu(paquete, pcb_a_enviar);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

void agregar_pcb_a_paquete_para_cpu(t_paquete* paquete, PCB* pcb) {
	agregar_registros_a_paquete(paquete, pcb->registrosCpu);
	agregar_int_a_paquete(paquete, pcb->id_proceso);
	agregar_int_a_paquete(paquete, pcb->estado);
	agregar_lista_a_paquete(paquete, pcb->lista_instrucciones);
	agregar_int_a_paquete(paquete, pcb->contador_instrucciones);
	agregar_lista_a_paquete(paquete, pcb->lista_segmentos);
	agregar_lista_a_paquete(paquete, pcb->lista_archivos_abiertos);
	agregar_valor_a_paquete(paquete, &pcb->estimacion_rafaga, sizeof(double));
	agregar_valor_a_paquete(paquete, &pcb->ready_timestamp, sizeof(double));
}
/*
void agregar_registros_a_paquete_cpu(t_paquete* paquete, registros_cpu* registrosCpu) {
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
*/

void agregar_lista_a_paquete(t_paquete* paquete, t_list* lista) {
	int tamanio = list_size(lista);
	agregar_int_a_paquete(paquete, tamanio);

	for(int i = 0; i < tamanio; i++) {
		void* elemento = list_get(lista, i);
		char* palabra = (char*)elemento;
		strtok(palabra, "$"); // Removemos el salto de linea
		log_debug(kernelLogger, "Agregando instruccion: %s, tamanio %zu", palabra, strlen(palabra));
		agregar_a_paquete(paquete, palabra, strlen(palabra));
	}

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

	agregar_registro4bytes_a_paquete(paquete, registrosCpu->AX);
	//log_error(kernelLogger, "REGISTRO AX: %d", registrosCpu->AX);
	agregar_registro4bytes_a_paquete(paquete, registrosCpu->BX);
	agregar_registro4bytes_a_paquete(paquete, registrosCpu->CX);
	agregar_registro4bytes_a_paquete(paquete, registrosCpu->DX);
	agregar_registro8bytes_a_paquete(paquete, registrosCpu->EAX);
	agregar_registro8bytes_a_paquete(paquete, registrosCpu->EBX);
	agregar_registro8bytes_a_paquete(paquete, registrosCpu->ECX);
	agregar_registro8bytes_a_paquete(paquete, registrosCpu->EDX);
	agregar_registro16bytes_a_paquete(paquete, registrosCpu->RAX);
	agregar_registro16bytes_a_paquete(paquete, registrosCpu->RBX);
	agregar_registro16bytes_a_paquete(paquete, registrosCpu->RCX);
	agregar_registro16bytes_a_paquete(paquete, registrosCpu->RDX);
}
void agregar_registro4bytes_a_paquete(t_paquete* paquete, char valor[4]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(int));
    paquete->buffer->size += sizeof(int);
}
void agregar_registro8bytes_a_paquete(t_paquete* paquete, char valor[8]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(long));
    paquete->buffer->size += sizeof(long);
}
void agregar_registro16bytes_a_paquete(t_paquete* paquete, char valor[16]) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long)*2);
    memcpy(paquete->buffer->stream + paquete->buffer->size, (void*)valor, sizeof(long)*2);
    paquete->buffer->size += sizeof(long)*2;
}


void agregar_long_a_paquete(t_paquete* paquete, long valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(long));
    paquete->buffer->size += sizeof(long);
}

void agregar_longlong_a_paquete(t_paquete* paquete, long long valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(long long));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(long long));
    paquete->buffer->size += sizeof(long long);
}

void agregar_a_lista(PCB* pcb, t_list* lista, sem_t m_sem) {
    sem_wait(&m_sem);
    list_add(lista, pcb);
    sem_post(&m_sem);
}

void agregar_pcb_a_paquete(t_paquete* paquete, PCB* pcb) {
	agregar_registros_a_paquete(paquete, pcb->registrosCpu);
	agregar_int_a_paquete(paquete, pcb->id_proceso);
	agregar_int_a_paquete(paquete, pcb->estado);
	agregar_lista_a_paquete(paquete, pcb->lista_instrucciones);
	agregar_int_a_paquete(paquete, pcb->contador_instrucciones);
	agregar_lista_a_paquete(paquete, pcb->lista_segmentos);
	agregar_lista_a_paquete(paquete, pcb->lista_archivos_abiertos);
	agregar_valor_a_paquete(paquete, &pcb->estimacion_rafaga, sizeof(double));
	agregar_valor_a_paquete(paquete, &pcb->ready_timestamp, sizeof(double));
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
