#include "cpu.h"

t_log* loggerCpu;
cpu_config_t* configCpu;

int conexionCpuKernel;
registros_cpu* registrosCpu;

bool hubo_interrupcion = false;

int main(int argc, char** argv) {
    loggerCpu = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, loggerCpu);
    cargar_config(config);

	pthread_mutex_init(&m_recibir_pcb,NULL);

	int conexionCpuMemoria = armar_conexion(config, MEMORIA, loggerCpu);
    enviar_codigo_operacion(conexionCpuMemoria, AUX_SOY_CPU);

    int servidorCPU = iniciar_servidor(config, loggerCpu);
    int clienteKernel = esperar_cliente(servidorCPU, loggerCpu);

    inicializar_registros();

    atender_kernel(clienteKernel);

    terminar_programa(conexionCpuKernel, loggerCpu, config);

    liberar_conexion(conexionCpuMemoria);
    free(registrosCpu);
    free(configCpu);

    return 0;
}

void atender_kernel(int clienteKernel){
	while(1) {
		pthread_mutex_lock(&m_recibir_pcb);
		PCB* pcb_a_ejecutar = recibir_pcb(clienteKernel);

		ejecutar_proceso(pcb_a_ejecutar, clienteKernel);
		pthread_mutex_unlock(&m_recibir_pcb);
	}
	return;

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
	strcpy(registrosCpu->AX, "");
    strcpy(registrosCpu->BX, "");
    strcpy(registrosCpu->CX, "");
    strcpy(registrosCpu->DX, "");
    strcpy(registrosCpu->EAX, "");
    strcpy(registrosCpu->EBX, "");
    strcpy(registrosCpu->ECX, "");
    strcpy(registrosCpu->EDX, "");
    strcpy(registrosCpu->RAX, "");
    strcpy(registrosCpu->RBX, "");
    strcpy(registrosCpu->RCX, "");
    strcpy(registrosCpu->RDX, "");

}

void procesar_instruccion(void * clienteAceptado) {
	int clienteKernel = (int) (intptr_t)clienteAceptado;

	codigo_operacion codigoOperacion = recibir_operacion(clienteKernel);
	if (codigoOperacion != OP_EXECUTE_PCB) {
		log_error(loggerCpu, "No se recibió un codigo de operacion de tipo pcb. Codigo recibido %d", codigoOperacion);
		abort();
	}

	PCB* pcb = recibir_pcb(clienteKernel);
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
	ejecutar_proceso(pcb, clienteKernel);
	//free(pcb);

	return;
}

PCB* recibir_pcb(int clienteAceptado) {
	PCB* pcb = malloc(sizeof(PCB));

	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

	recibir_operacion(clienteAceptado);
	buffer = recibir_buffer(&tamanio, clienteAceptado);

	pcb->registrosCpu = malloc(sizeof(registros_cpu));
	strcpy(pcb->registrosCpu->AX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->BX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->CX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->DX, leer_registro_4_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EAX,  leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EBX,  leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->ECX,  leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->EDX,  leer_registro_8_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RAX,  leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RBX,  leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RCX,  leer_registro_16_bytes(buffer, &desplazamiento));
	strcpy(pcb->registrosCpu->RDX,  leer_registro_16_bytes(buffer, &desplazamiento));

	pcb->id_proceso = leer_int(buffer, &desplazamiento);

	pcb->estado = leer_int(buffer, &desplazamiento);

	pcb->lista_instrucciones = leer_string_array(buffer, &desplazamiento);

	pcb->contador_instrucciones = leer_int(buffer, &desplazamiento);

	pcb->lista_segmentos = leer_string_array(buffer, &desplazamiento);

	/*
	pcb->lista_archivos_abiertos = list_create();
	int cantidad_de_archivos = leer_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_archivos; i++) {
			t_archivo_abierto* archivo_abierto = malloc(sizeof(t_archivo_abierto));

		    archivo_abierto->nombreArchivo = leer_string(buffer, &desplazamiento);
		    archivo_abierto->puntero = leer_int(buffer, &desplazamiento);

		    list_add(pcb->lista_archivos_abiertos, archivo_abierto);
		    free(archivo_abierto);
	}

*/
	pcb->estimacion_rafaga = leer_double(buffer, &desplazamiento);
	pcb->ready_timestamp = leer_double(buffer, &desplazamiento);


	return pcb;
}

void ejecutar_proceso(PCB* pcb, int clienteKernel) {

	cargar_registros(pcb);

	// ¿Por que se le hace malloc?
	char* instruccion;
	char** instruccion_decodificada;

	t_list* data_instruccion; // Array para los parametros que necesite una instruccion

	int cantidad_instrucciones = list_size(pcb->lista_instrucciones);
	int posicion_actual = pcb->contador_instrucciones;
	codigo_operacion ultimaOperacion = -1;

    while ((posicion_actual < cantidad_instrucciones) && !hubo_interrupcion) {
	    instruccion = string_duplicate((char *)list_get(pcb->lista_instrucciones, pcb->contador_instrucciones));
		instruccion_decodificada = decode_instruccion(instruccion, loggerCpu);

        log_info(loggerCpu, "Ejecutando instruccion: %s", instruccion_decodificada[0]);
        ultimaOperacion = ejecutar_instruccion(instruccion_decodificada, pcb);

        if (!hubo_interrupcion) {
			pcb->contador_instrucciones++;
			posicion_actual++;
        }

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

	//mostrar_pcb(pcb);

	enviar_pcb_desalojado_a_kernel(pcb, clienteKernel, ultimaOperacion);
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
	log_trace(loggerCpu, "ESTIMACION HHRN: %f", pcb->estimacion_rafaga);
	log_trace(loggerCpu, "TIMESTAMP EN EL QUE EL PROCESO LLEGO A READY POR ULTIMA VEZ: %f", pcb->ready_timestamp);
}

void iterator(char* value) {
    log_info(loggerCpu, "%s ", value);
}


void cargar_registros(PCB* pcb) {
	strcpy(registrosCpu->AX, pcb->registrosCpu->AX);
	strcpy(registrosCpu->BX, pcb->registrosCpu->BX);
	strcpy(registrosCpu->CX, pcb->registrosCpu->CX);
	strcpy(registrosCpu->DX, pcb->registrosCpu->DX);
	strcpy(registrosCpu->EAX,  pcb->registrosCpu->EAX);
	strcpy(registrosCpu->EBX,  pcb->registrosCpu->EBX);
	strcpy(registrosCpu->ECX,  pcb->registrosCpu->ECX);
	strcpy(registrosCpu->EDX,  pcb->registrosCpu->EDX);
	strcpy(registrosCpu->RAX,  pcb->registrosCpu->RAX);
	strcpy(registrosCpu->RBX,  pcb->registrosCpu->RBX);
	strcpy(registrosCpu->RCX,  pcb->registrosCpu->RCX);
	strcpy(registrosCpu->RDX,  pcb->registrosCpu->RDX);
}

void guardar_contexto_de_ejecucion(PCB* pcb) {
	strcpy(pcb->registrosCpu->AX, registrosCpu->AX);
    strcpy(pcb->registrosCpu->BX, registrosCpu->BX);
    strcpy(pcb->registrosCpu->CX, registrosCpu->CX);
    strcpy(pcb->registrosCpu->DX, registrosCpu->DX);
    strcpy(pcb->registrosCpu->EAX,  registrosCpu->EAX);
    strcpy(pcb->registrosCpu->EBX,  registrosCpu->EBX);
    strcpy(pcb->registrosCpu->ECX,  registrosCpu->ECX);
    strcpy(pcb->registrosCpu->EDX,  registrosCpu->EDX);
    strcpy(pcb->registrosCpu->RAX,  registrosCpu->RAX);
    strcpy(pcb->registrosCpu->RBX,  registrosCpu->RBX);
    strcpy(pcb->registrosCpu->RCX,  registrosCpu->RCX);
    strcpy(pcb->registrosCpu->RDX,  registrosCpu->RDX);

//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro AX: %s", truncar_string(pcb->registrosCpu->AX,4));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro BX: %s", truncar_string(pcb->registrosCpu->BX,4));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro CX: %s", truncar_string(pcb->registrosCpu->CX,4));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro DX: %s", truncar_string(pcb->registrosCpu->DX,4));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro EAX: %s", truncar_string(pcb->registrosCpu->EAX,8));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro ECX: %s", truncar_string(pcb->registrosCpu->ECX,8));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro EBX: %s", truncar_string(pcb->registrosCpu->EBX,8));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro EDX: %s", truncar_string(pcb->registrosCpu->EDX,8));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro RAX: %s", truncar_string(pcb->registrosCpu->RAX,16));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro RBX: %s", truncar_string(pcb->registrosCpu->RBX,16));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro RCX: %s", truncar_string(pcb->registrosCpu->RCX,16));
//	log_trace(loggerCpu, "Guardando contexto de ejecucion: Registro RDX: %s", truncar_string(pcb->registrosCpu->RDX,16));

}

int ejecutar_instruccion(char** instruccion, PCB* pcb) {

	char* texto = strtok(instruccion[0], "$");
	int operacion = keyFromString(texto);

	if (operacion == -1) {
		log_warning(loggerCpu, "Desconocemos la instruccion %s", instruccion[0]);

		return -1;
	}

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
			// instruccion_mov_in(instruccion[1],instruccion[2],pcb);
			break;
		case I_MOV_OUT:
			// MOV_OUT (Dirección Lógica, Registro)
			// instruccion_mov_out(instruccion[1],instruccion[2],pcb);
			break;
		case I_IO:
			// I/O (Tiempo)
			instruccion_io(instruccion[1]);
			break;
		case I_F_OPEN:
			// F_OPEN (Nombre Archivo)
			instruccion_f_open(instruccion[1]);
			hubo_interrupcion = true;
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
			hubo_interrupcion = true;
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
			hubo_interrupcion = true;
			break;
	}
	return operacion;
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
		strcpy(registrosCpu->AX, valor);
	} else if (strcmp(registro, "BX") == 0) {
		strcpy(registrosCpu->BX, valor);
	} else if (strcmp(registro, "CX") == 0) {
		strcpy(registrosCpu->CX, valor);
	} else if (strcmp(registro, "DX") == 0) {
		strcpy(registrosCpu->DX, valor);
	} else if (strcmp(registro, "EAX") == 0) {
		strcpy(registrosCpu->EAX, valor);
	} else if (strcmp(registro, "EBX") == 0) {
		strcpy(registrosCpu->EBX, valor);
	} else if (strcmp(registro, "ECX") == 0) {
		strcpy(registrosCpu->ECX, valor);
	} else if (strcmp(registro, "EDX") == 0) {
		strcpy(registrosCpu->EDX, valor);
	} else if (strcmp(registro, "RAX") == 0) {
		strcpy(registrosCpu->RAX, valor);
	} else if (strcmp(registro, "RBX") == 0) {
		strcpy(registrosCpu->RBX, valor);
	} else if (strcmp(registro, "RCX") == 0) {
		strcpy(registrosCpu->RCX, valor);
	} else if (strcmp(registro, "RDX") == 0) {
		strcpy(registrosCpu->RDX, valor);
	}

	//sleep(configCpu->RETARDO_INSTRUCCION/1000);
}

// void instruccion_set(char* registro,char* valor) {

// 	int set_valor = atoi(valor);

// 	void* registro_cpu = get_registro_cpu(registro, registrosCpu);

// 	if(registro_cpu!=-1){
// 		registro_cpu = set_valor;
// 	} else {
// 		log_error(loggerCpu, "Registro de CPU no reconocido.");
// 		hubo_interrupcion=true;
// 		return;
// 	}

// 	usleep(configCpu->RETARDO_INSTRUCCION*1000); // De microsegundos a nanosegundos
// 	free(set_valor);
// 	free(registro_cpu);
// }

void instruccion_mov_in(char* registro, char* dir_logica, PCB* pcb) {
	/*
	 * Lee el valor de memoria corresponfiente a la Direccion Logica y lo almacena en el registro

	t_segmento* segmento;
	void* dir_fisica = get_dir_fisica(segmento, dir_logica, configCpu->TAM_MAX_SEGMENTO);
	if(dir_fisica!=-1){
		void* registro_cpu = get_registro_cpu(registro, registrosCpu);

		if(registro_cpu!=-1){
			registro_cpu = *&dir_fisica; // Valor en la direccion fisica
			log_info(loggerCpu, "Se asigno el valor de memoria corresponfiente a la Direccion Logica %s y se almaceno en el registro %s", dir_logica, registro);
		} else {
			log_error(loggerCpu, "Registro de CPU no reconocido.");
			hubo_interrupcion=true;
		}
		free(registro_cpu);
	} else {
		hubo_interrupcion=true;
		log_error(loggerCpu, ERROR_SEGMENTATION_FAULT, pcb->id_proceso, segmento->id, segmento->direccionBase, segmento->size);
	}
	free(segmento);
	free(dir_fisica);
	*/
}

void instruccion_mov_out(char* dir_logica, char* registro, PCB* pcb) {
	/*
	 * Lee el valor del Registro y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica.
	t_segmento* segmento;
	void* dir_fisica = get_dir_fisica(segmento, dir_logica, configCpu->TAM_MAX_SEGMENTO);

	if(dir_fisica!=-1){
		void* registro_cpu = get_registro_cpu(registro, registrosCpu);

		if(registro_cpu!=-1){
			*&dir_fisica = registro_cpu;
			log_info(loggerCpu, "Se leyo el valor del registro %s y se almaceno en la direccion fisica de memoria %s", registro, dir_logica);
		} else {
			log_error(loggerCpu, "Registro de CPU no reconocido.");
			hubo_interrupcion=true;
		}
		free(registro_cpu);
	} else {
		hubo_interrupcion=true;
		// flag_seg_fault=true;
		log_error(loggerCpu, ERROR_SEGMENTATION_FAULT, pcb->id_proceso, segmento->id, segmento->direccionBase, segmento->size);
	}
	free(segmento);
	free(dir_fisica);
	*/
}

void enviar_pcb_desalojado_a_kernel(PCB* pcb, int socket, codigo_operacion codigo){

	envio_pcb_a_kernel_con_codigo(socket, pcb, codigo);
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
	// agregar_lista_a_paquete(paquete, pcb->lista_archivos_abiertos);
	agregar_registros_a_paquete_para_kernel(paquete, pcb->registrosCpu);
	agregar_valor_a_paquete(paquete, &pcb->estimacion_rafaga, sizeof(double));
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


void agregar_valor_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio);
    memcpy(paquete->buffer->stream + paquete->buffer->size, valor, tamanio);
    paquete->buffer->size += tamanio;
}
//
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
