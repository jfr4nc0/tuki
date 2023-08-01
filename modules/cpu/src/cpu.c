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
	    codigo_operacion codDePrueba1 = recibir_operacion(clienteKernel);
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
		if (instruccion_decodificada[0] != NULL) {
			log_info(loggerCpu, "Ejecutando instruccion: %s", instruccion);
			ultimaOperacion = ejecutar_instruccion(instruccion_decodificada, pcb);

			if (ultimaOperacion == I_F_READ || ultimaOperacion == I_F_WRITE) {
				// Reescribo la instruccion usando dir fisica en vez de logica
				char* instruccionConvertida = convertir_dir_logica_a_fisica(pcb, instruccion_decodificada[2]);
				instruccion_decodificada[2] = instruccionConvertida;
				instruccion = encode_instruccion(instruccion_decodificada);
				list_replace(pcb->lista_instrucciones, pcb->contador_instrucciones, instruccion);
				free(instruccionConvertida);
			}

			if (!hubo_interrupcion) {
				pcb->contador_instrucciones++;
				posicion_actual++;
			}

			log_info(loggerCpu, "PROGRAM COUNTER: %d", pcb->contador_instrucciones);
		}
    }

    log_info(loggerCpu, "Se salió de la ejecucion en la instrucción %s. Guardando el contexto de ejecucion...", instruccion);
    guardar_contexto_de_ejecucion(pcb);

    free(instruccion);
    free(instruccion_decodificada);

	// Si hubo interrupcion de algun tipo se lo comunico a kernel pero sacamos
	if (hubo_interrupcion) {
		hubo_interrupcion = false;
	}

	//mostrar_pcb(pcb, loggerCpu);

	enviar_pcb(clienteKernel, pcb, ultimaOperacion, loggerCpu);
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

	char* texto = strtok(instruccion[0], "\n");
	int operacion = keyFromString(texto);

	if (operacion == -1) {
		log_warning(loggerCpu, "Desconocemos la instruccion %s", instruccion[0]);

		return -1;
	}

	switch(operacion) {
		case I_YIELD:
		case I_F_OPEN:
		case I_EXIT:
		case I_F_CLOSE:
		case I_F_SEEK:
		case I_F_READ:
		case I_F_WRITE:
		case I_TRUNCATE:
			hubo_interrupcion = true;
		break;
	}

	switch(operacion) {
		case I_SET: {
			// SET (Registro, Valor)
			int retardo = configCpu->RETARDO_INSTRUCCION;
			intervalo_de_pausa(retardo);
			instruccion_set(instruccion[1],instruccion[2]);
			//log_info(loggerCpu, "REGISTRO AX: %s", registrosCpu->AX);
			break;
		}
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
			break;
		case I_WAIT:
			// WAIT (Recurso)
			break;
		case I_SIGNAL:
			// SIGNAL (Recurso)
			break;
		case I_CREATE_SEGMENT:
			// CREATE_SEGMENT (Id del Segmento, Tamaño)
			break;
		case I_DELETE_SEGMENT:
			// DELETE_SEGMENT (Id del Segmento)
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

char* convertir_dir_logica_a_fisica(PCB *pcb, char* dirLogicaTexto) {
	uint32_t numeroSegmento, offset, tamanioSegmento;

	char* endptr; // Puntero para manejar errores en la conversión
	uint32_t dirLogica = strtoul(dirLogicaTexto, &endptr, 10); // Convertir la cadena a un valor numérico uint32_t

	// Verificar si hubo algún error en la conversión
	if (*endptr != '\0') {
		log_error(loggerCpu, "El valor no representa un número válido de direccion logica");
	}

	void* dirFisica = obtener_puntero_direccion_fisica(pcb, dirLogica, &numeroSegmento, &offset, &tamanioSegmento);
	log_info(loggerCpu, "COnversion de memoria dirLogica <%s> (en numero %d) a fisica <%p> (convertida a texto es: %s)",
		dirLogicaTexto, dirLogica, dirFisica, (char*)dirFisica);
    return (char*)dirFisica;
}

void* obtener_puntero_direccion_fisica(PCB *pcb,uint32_t dirLogica, uint32_t *numeroSegmento, uint32_t *offset, uint32_t *tamanioSegmento){
    uint32_t tam_max_segmento;
    tam_max_segmento = configCpu->TAM_MAX_SEGMENTO;
    uint32_t numero_de_segmento = (dirLogica / tam_max_segmento);
    *offset = (uint32_t) dirLogica % tam_max_segmento;
    void* base = obtener_base_segmento(pcb, numero_de_segmento, tamanioSegmento);
    void* direccionFisica = calcular_direccion(base, (size_t)(*offset));
    return direccionFisica;
}

void* obtener_base_segmento(PCB *pcb, uint32_t numeroSegmento,  uint32_t *tamanio){
    void* base;
    uint32_t i = 0;

    int cantidadSegmentos = list_size(pcb->lista_segmentos);
   	t_segmento* segmentoTabla;

   	if (cantidadSegmentos == 0) {
   		log_error(loggerCpu, "Error no hay bases de segmentos en el pcb, fijarse si memoria le dio a kernel la base de segmentos");
   		return 0;
   	}

   	while(i < cantidadSegmentos){
   		segmentoTabla = list_get(pcb->lista_segmentos, i);
   		if(segmentoTabla->id == numeroSegmento){
			log_info(loggerCpu, "Base encontrada, en uint32: %p, en char %s", segmentoTabla->direccionBase, (char*)segmentoTabla->direccionBase);
			return segmentoTabla->direccionBase;
   		}
   		i++;
   	}
	log_error(loggerCpu, "DIR LOGICA NO encontrada a partir de nro de segmento <%d>", numeroSegmento);
    return NULL;
}
