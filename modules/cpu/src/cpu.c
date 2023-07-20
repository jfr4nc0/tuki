#include "cpu.h"

t_log* loggerCpu;
cpu_config_t* configCpu;

int conexionCpuKernel;
registros_cpu* registrosCpu;

bool hubo_interrupcion = false;
codigo_operacion ultimaOperacion;

int main(int argc, char** argv) {
    loggerCpu = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, loggerCpu);
    cargar_config(config);

    int conexionCpuMemoria = armar_conexion(config, MEMORIA, loggerCpu);
    enviar_codigo_operacion(conexionCpuMemoria, AUX_SOY_CPU);

    inicializar_registros();

    int servidorCPU = iniciar_servidor(config, loggerCpu);

    while (1) {
    	int clienteKernel = esperar_cliente(servidorCPU, loggerCpu);
	    //pthread_t hilo_ejecucion;
	    pthread_t hilo_dispatcher;
		//pthread_create(&hilo_ejecucion, NULL, (void *) procesar_instruccion, int servidorCPU); //  TODO: Avisar posibles errores o si el kernel se desconecto.
		pthread_create(&hilo_dispatcher, NULL, (void *) procesar_instruccion, (void*) (intptr_t) clienteKernel);

		pthread_join(hilo_dispatcher, NULL);
    }

    terminar_programa(conexionCpuKernel, loggerCpu, config);

    liberar_conexion(conexionCpuMemoria);
    free(registrosCpu);
    free(configCpu);

    return 0;
}

void cargar_config(t_config* config) {
	configCpu = malloc(sizeof(cpu_config_t));
	configCpu->RETARDO_INSTRUCCION = extraer_int_de_config(config, "RETARDO_INSTRUCCION", loggerCpu);
	configCpu->IP_MEMORIA = extraer_string_de_config(config, "IP_MEMORIA", loggerCpu);
	configCpu->PUERTO_MEMORIA = extraer_int_de_config(config, "PUERTO_MEMORIA", loggerCpu);
	configCpu->PUERTO_ESCUCHA = extraer_int_de_config(config, "PUERTO_ESCUCHA", loggerCpu);
	configCpu->TAM_MAX_SEGMENTO = extraer_int_de_config(config, "TAM_MAX_SEGMENTO", loggerCpu);
}

void inicializar_registros() {
	registrosCpu = malloc(sizeof(registros_cpu));
	registrosCpu->AX = 0;
	registrosCpu->BX = 0;
	registrosCpu->CX = 0;
	registrosCpu->DX = 0;
	registrosCpu->EAX = 0;
	registrosCpu->EBX = 0;
	registrosCpu->ECX = 0;
	registrosCpu->EDX = 0;
	registrosCpu->RAX = 0;
	registrosCpu->RBX = 0;
	registrosCpu->RCX = 0;
	registrosCpu->RDX = 0;
}

void procesar_instruccion(void * clienteAceptado) {
	int clienteKernel = (int) (intptr_t)clienteAceptado;

	codigo_operacion codigoOperacion = recibir_operacion(clienteKernel);
	if (codigoOperacion != OP_EXECUTE_PCB) {
		log_error(loggerCpu, "No se recibió un codigo de operacion de tipo pcb. Codigo recibido %d", codigoOperacion);
		abort();
	}

	PCB* pcb = recibir_pcb(clienteKernel);

	ejecutar_proceso(pcb, clienteKernel);
	free(pcb);

	return;
}

PCB* recibir_pcb(int clienteAceptado) {
	PCB* pcb = malloc(sizeof(PCB));

	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

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

	pcb->processor_burst = leer_double(buffer, &desplazamiento);
	pcb->ready_timestamp = leer_double(buffer, &desplazamiento);

	return pcb;
}

void ejecutar_proceso(PCB* pcb, int clienteKernel) {

	cargar_registros(pcb);

	char* instruccion = malloc(sizeof(char*));
	char** instruccion_decodificada = malloc(sizeof(char*));

	int cantidad_instrucciones = list_size(pcb->lista_instrucciones);
	int posicion_actual = 0;

    // Se tienen en cuenta las posibles causas de interrupciones al proceso
    // 1. Termina el proceso (f_eop)
    // 2. Se produce una interrupcion (f_interruption)
    // 3. IO Exception (f_io)
    // 4. Page Fault Exception (f_pagefault)
    // 5. Segmentation Fault (f_segfault)
    // Denotamos con f_<var> (f: Flag)

	// while(f_eop!=1 && f_interruption!=1 && f_io!=1 && f_pagefault!=1 && f_segfault!=1) {

    while ((posicion_actual < cantidad_instrucciones) && !hubo_interrupcion) {
	    instruccion = string_duplicate((char *)list_get(pcb->lista_instrucciones, pcb->contador_instrucciones));
		instruccion_decodificada = decode_instruccion(instruccion);

        log_info(loggerCpu, "Ejecutando instruccion: %s", instruccion_decodificada[0]);
        ejecutar_instruccion(instruccion_decodificada, pcb, clienteKernel);

        // Evaluar si la instruccion genero una excepcion "f_pagefault"
        // Caso afirmativo => No se actualiza el program counter del pcb
        pcb->contador_instrucciones++;
		posicion_actual++;

        log_info(loggerCpu, "PROGRAM COUNTER: %d", pcb->contador_instrucciones);
    }

    log_info(loggerCpu, "Se salió de la ejecucion en la instrucción %s. Guardando el contexto de ejecucion...", instruccion);
    guardar_contexto_de_ejecucion(pcb);

    free(instruccion);
    free(instruccion_decodificada);

	// Si hubo interrupcion de algun tipo se lo comunico a kernel pero sacamos
	if (hubo_interrupcion) {
		hubo_interrupcion = false;
	}
	log_error(loggerCpu, "REGISTRO AX: %s", pcb->registrosCpu->AX);
	mostrar_pcb(pcb);
	enviar_pcb_desalojado_a_kernel(pcb, clienteKernel);
}


void mostrar_pcb(PCB* pcb){
	log_trace(loggerCpu, "PID: %d", pcb->id_proceso);
	char* estado = nombres_estados[pcb->estado];
	log_trace(loggerCpu, "ESTADO: %s", estado);
	log_trace(loggerCpu, "INSTRUCCIONES A EJECUTAR: ");
	list_iterate(pcb->lista_instrucciones, (void*) iterator);
	log_trace(loggerCpu, "PROGRAM COUNTER: %d", pcb->contador_instrucciones);
	log_trace(loggerCpu, "Registro AX: %s", pcb->registrosCpu->AX);
	log_trace(loggerCpu, "Registro BX: %s", pcb->registrosCpu->BX);
	log_trace(loggerCpu, "Registro CX: %s", pcb->registrosCpu->CX);
	log_trace(loggerCpu, "Registro DX: %s", pcb->registrosCpu->DX);
	log_trace(loggerCpu, "Registro EAX: %s", pcb->registrosCpu->EAX);
	log_trace(loggerCpu, "Registro EBX: %s", pcb->registrosCpu->EBX);
	log_trace(loggerCpu, "Registro ECX: %s", pcb->registrosCpu->ECX);
	log_trace(loggerCpu, "Registro EDX: %s", pcb->registrosCpu->EDX);
	log_trace(loggerCpu, "Registro RAX: %s", pcb->registrosCpu->RAX);
	log_trace(loggerCpu, "Registro RBX: %s", pcb->registrosCpu->RBX);
	log_trace(loggerCpu, "Registro RCX: %s", pcb->registrosCpu->RCX);
	log_trace(loggerCpu, "Registro RDX: %s", pcb->registrosCpu->RDX);
	log_trace(loggerCpu, "LISTA SEGMENTOS: ");
	list_iterate(pcb->lista_segmentos, (void*) iterator);
	log_trace(loggerCpu, "LISTA ARCHIVOS ABIERTOS: ");
	list_iterate(pcb->lista_archivos_abiertos, (void*) iterator);
	log_trace(loggerCpu, "ESTIMACION HHRN: %f", pcb->processor_burst);
	log_trace(loggerCpu, "TIMESTAMP EN EL QUE EL PROCESO LLEGO A READY POR ULTIMA VEZ: %f", pcb->ready_timestamp);
}
void iterator(char* value) {
    log_info(loggerCpu, "%s ", value);
}


void cargar_registros(PCB* pcb) {
	registrosCpu->AX = pcb->registrosCpu->AX;
	registrosCpu->BX = pcb->registrosCpu->BX;
	registrosCpu->CX = pcb->registrosCpu->CX;
	registrosCpu->DX = pcb->registrosCpu->DX;
	registrosCpu->AX = pcb->registrosCpu->EAX;
	registrosCpu->BX = pcb->registrosCpu->EBX;
	registrosCpu->CX = pcb->registrosCpu->ECX;
	registrosCpu->DX = pcb->registrosCpu->EDX;
	registrosCpu->AX = pcb->registrosCpu->RAX;
	registrosCpu->BX = pcb->registrosCpu->RBX;
	registrosCpu->CX = pcb->registrosCpu->RCX;
	registrosCpu->DX = pcb->registrosCpu->RDX;
}

char** decode_instruccion(char* linea_a_parsear) {
	char** instruccion = string_split(linea_a_parsear, " ");
	if(instruccion[0] == NULL) {
	    log_info(loggerCpu, "Se ignora linea vacía.");
	}
	return instruccion;
}

void guardar_contexto_de_ejecucion(PCB* pcb) {
	pcb->registrosCpu->AX = registrosCpu->AX;
	pcb->registrosCpu->BX = registrosCpu->BX;
	pcb->registrosCpu->CX = registrosCpu->CX;
	pcb->registrosCpu->DX = registrosCpu->DX;
	pcb->registrosCpu->EAX = registrosCpu->EAX;
	pcb->registrosCpu->EBX = registrosCpu->EBX;
	pcb->registrosCpu->ECX = registrosCpu->ECX;
	pcb->registrosCpu->EDX = registrosCpu->EDX;
	pcb->registrosCpu->RAX = registrosCpu->RAX;
	pcb->registrosCpu->RBX = registrosCpu->RBX;
	pcb->registrosCpu->RCX = registrosCpu->RCX;
	pcb->registrosCpu->RDX = registrosCpu->RDX;

}

void ejecutar_instruccion(char** instruccion, PCB* pcb, int clienteKernel) {

	int operacion = keyFromString(instruccion[0]);

	if (operacion == -1) {
		log_warning(loggerCpu, "Desconocemos la instruccion %s", instruccion[0]);

		return;
	}

	ultimaOperacion = operacion;

	switch(operacion) {
		case I_SET:
			// SET (Registro, Valor)
			int retardo = configCpu->RETARDO_INSTRUCCION;
			intervalo_de_pausa(retardo);
			instruccion_set(instruccion[1],instruccion[2]);
			//log_info(loggerCpu, "REGISTRO AX: %s", registrosCpu->AX);
			break;
		case I_MOV_IN:
			// MOV_IN (Registro, Dirección Lógica)
			instruccion_mov_in(instruccion[1],instruccion[2]);
			break;
		case I_MOV_OUT:
			// MOV_OUT (Dirección Lógica, Registro)
			instruccion_mov_out(instruccion[1],instruccion[2]);
			break;
		case I_IO:
			// I/O (Tiempo)
			instruccion_io(instruccion[1]);
			break;
		case I_F_OPEN:
			// F_OPEN (Nombre Archivo)
			instruccion_f_open(instruccion[1]);
			break;
		case I_F_CLOSE:
			// F_CLOSE (Nombre Archivo)
			instruccion_f_close(instruccion[1]);
			break;
		case I_F_SEEK:
			// F_SEEK (Nombre Archivo, Posición)
			instruccion_f_seek(instruccion[1],instruccion[2]);
			break;
		case I_F_READ:
			// F_READ (Nombre Archivo, Dirección Lógica, Cantidad de Bytes)
			instruccion_f_read(instruccion[1],instruccion[2],instruccion[3]);
			break;
		case I_F_WRITE:
			// F_WRITE (Nombre Archivo, Dirección Lógica, Cantidad de bytes)
			instruccion_f_write(instruccion[1],instruccion[2],instruccion[3]);
			break;
		case I_TRUNCATE:
			// F_TRUNCATE (Nombre Archivo, Tamaño)
			instruccion_f_truncate(instruccion[1],instruccion[2]);
			break;
		case I_WAIT:
			// WAIT (Recurso)
			instruccion_wait(instruccion[1]);
			break;
		case I_SIGNAL:
			// SIGNAL (Recurso)
			instruccion_signal(instruccion[1]);
			break;
		case I_CREATE_SEGMENT:
			// CREATE_SEGMENT (Id del Segmento, Tamaño)
			instruccion_create_segment(instruccion[1],instruccion[2]);
			break;
		case I_DELETE_SEGMENT:
			// DELETE_SEGMENT (Id del Segmento)
			instruccion_delete_segment(instruccion[1]);
			break;
		case I_YIELD:
			//enviar_pcb_desalojado_a_kernel(pcb, clienteKernel);

			hubo_interrupcion = true;
			break;
		case I_EXIT:
			instruccion_exit();
			hubo_interrupcion = true;

			break;
	}
}

/************** INSTRUCCIONES ***************************/

void instruccion_set(char* registro,char* valor) {

	/*
	int set_valor = atoi(valor);
	if(set_valor == 0){
		log_info(loggerCpu, "Hubo un error.");
	}
	*/

	if (strcmp(registro, "AX") == 0) {
		registrosCpu->AX = valor;
	} else if (strcmp(registro, "BX") == 0) {
		registrosCpu->BX = valor;
	} else if (strcmp(registro, "CX") == 0) {
		registrosCpu->CX = valor;
	} else if (strcmp(registro, "DX") == 0) {
		registrosCpu->DX = valor;
	} else if (strcmp(registro, "EAX") == 0) {
		registrosCpu->EAX = valor;
	} else if (strcmp(registro, "EBX") == 0) {
		registrosCpu->EBX = valor;
	} else if (strcmp(registro, "ECX") == 0) {
		registrosCpu->ECX = valor;
	} else if (strcmp(registro, "EDX") == 0) {
		registrosCpu->EDX = valor;
	} else if (strcmp(registro, "RAX") == 0) {
		registrosCpu->RAX = valor;
	} else if (strcmp(registro, "RBX") == 0) {
		registrosCpu->RBX = valor;
	} else if (strcmp(registro, "RCX") == 0) {
		registrosCpu->RCX = valor;
	} else if (strcmp(registro, "RDX") == 0) {
		registrosCpu->RDX = valor;
	}

	//sleep(configCpu->RETARDO_INSTRUCCION/1000);
}

void enviar_pcb_desalojado_a_kernel(PCB* pcb, int socket){

	envio_pcb_a_kernel_con_codigo(socket, pcb, DESALOJO_YIELD);
}

void envio_pcb_a_kernel_con_codigo(int conexion, PCB* pcb, codigo_operacion codigo) {
	t_paquete* paquete = crear_paquete(codigo);
	agregar_pcb_a_paquete(paquete, pcb);
	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

// Repetidas
void agregar_pcb_a_paquete(t_paquete* paquete, PCB* pcb) {
	agregar_int_a_paquete(paquete, pcb->id_proceso);
	agregar_int_a_paquete(paquete, pcb->estado);

	agregar_lista_a_paquete(paquete, pcb->lista_instrucciones);

	agregar_int_a_paquete(paquete, pcb->contador_instrucciones);
	agregar_lista_a_paquete(paquete, pcb->lista_segmentos);
	agregar_lista_a_paquete(paquete, pcb->lista_archivos_abiertos);
	agregar_registros_a_paquete_para_kernel(paquete, pcb->registrosCpu);
	agregar_valor_a_paquete(paquete, &pcb->processor_burst, sizeof(double));
	agregar_valor_a_paquete(paquete, &pcb->ready_timestamp, sizeof(double));
}
void agregar_int_a_paquete(t_paquete* paquete, int valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(int));
    paquete->buffer->size += sizeof(int);
}

void agregar_lista_a_paquete(t_paquete* paquete, t_list* lista) {
	int tamanio = list_size(lista);
	agregar_int_a_paquete(paquete, tamanio);

	for(int i = 0; i < tamanio; i++) {
		void* elemento = list_get(lista, i);
		char* palabra = (char*)elemento;
		strtok(palabra, "$"); // Removemos el salto de linea
		log_debug(loggerCpu, "Agregando instruccion: %s, tamanio %zu", palabra, strlen(palabra));
		agregar_a_paquete(paquete, palabra, strlen(palabra));
	}

}
void agregar_registros_a_paquete_para_kernel(t_paquete* paquete, registros_cpu* registrosCpu) {
	 agregar_registro4bytes_a_paquete(paquete, registrosCpu->AX);
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
void agregar_registro4bytes_a_paquete(t_paquete* paquete, char* valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(4));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(4));
    paquete->buffer->size += sizeof(4);
}
void agregar_registro8bytes_a_paquete(t_paquete* paquete, char* valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(8));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(8));
    paquete->buffer->size += sizeof(8);
}
void agregar_registro16bytes_a_paquete(t_paquete* paquete, char* valor) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(16));
    memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(16));
    paquete->buffer->size += sizeof(16);
}
void agregar_valor_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);
    paquete->buffer->size += tamanio;
}
//
void instruccion_mov_in(char* registro,char* dir_logica) {

}
void instruccion_mov_out(char* dir_logica,char* registro) {

}
void instruccion_io(char* tiempo) {

}
void instruccion_f_open(char* nombre_archivo) {

}
void instruccion_f_close(char* nombre_archivo) {

}
void instruccion_f_seek(char* nombre_archivo, char* posicion) {

}
void instruccion_f_read(char* nombre_archivo, char* dir_logica, char* cant_bytes) {

}
void instruccion_f_write(char* nombre_archivo, char* dir_logica, char* cant_bytes) {

}
void instruccion_f_truncate(char* nombre_archivo,char* tamanio) {

}
void instruccion_wait(char* recurso) {

}
void instruccion_signal(char* recurso) {

}
void instruccion_create_segment(char* id_segmento, char* tamanio) {

}
void instruccion_delete_segment(char* id_segmento) {

}
void instruccion_exit() {

}

/*
 * Devuelve el pcb a kernel porque terminó de ejecutar el proceso
 */
void devolver_pcb_kernel(PCB* pcb, int conexion, codigo_operacion codOperacion) {
	enviar_operacion(conexion, codOperacion, sizeof(PCB), pcb);
}
