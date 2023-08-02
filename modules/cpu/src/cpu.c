#include "cpu.h"

t_log* loggerCpu;
cpu_config_t* configCpu;

int conexionCpuMemoria;
int conexionCpuKernel;
registros_cpu* registrosCpu;

bool hubo_interrupcion = false;

int main(int argc, char** argv) {
    loggerCpu = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);
    t_config* config = iniciar_config(argv[1], loggerCpu);
    cargar_config(config);

    conexionCpuMemoria = armar_conexion(config, MEMORIA, loggerCpu);
	pthread_mutex_init(&m_recibir_pcb,NULL);
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

	//pcb->lista_segmentos = leer_string_array(buffer, &desplazamiento); TODO: no funciona

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

	//t_list* data_instruccion; // Array para los parametros que necesite una instruccion

	int cantidad_instrucciones = list_size(pcb->lista_instrucciones);
	int posicion_actual = pcb->contador_instrucciones;
	codigo_operacion ultimaOperacion = -1;

    while ((posicion_actual < cantidad_instrucciones) && !hubo_interrupcion) {
	    instruccion = string_duplicate((char *)list_get(pcb->lista_instrucciones, pcb->contador_instrucciones));
		instruccion_decodificada = decode_instruccion(instruccion, loggerCpu);

        log_info(loggerCpu, "PID: %u - Ejecutando: %s", pcb->id_proceso, instruccion_decodificada[0]);
        ultimaOperacion = ejecutar_instruccion(instruccion_decodificada, pcb);

        if (!hubo_interrupcion) {
			pcb->contador_instrucciones++;
			posicion_actual++;
        }

    }

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
}

int ejecutar_instruccion(char** instruccion, PCB* pcb) {

	char* instruccion_ = malloc(sizeof(char*));
	instruccion_ = strtok(instruccion[0], "\n");

	int operacion = keyFromString(instruccion_);

	if (operacion == -1) {
		log_error(loggerCpu, "Desconocemos la instruccion %s", instruccion[0]);

		return -1;
	}

	switch(operacion) {
		// Si hay interrupcion no hago nada y se lo devuelvo a kernel
		case I_F_OPEN:
		case I_YIELD:
		case I_EXIT:
		case I_F_CLOSE:
		case I_F_SEEK:
		case I_F_READ:
		case I_F_WRITE:
		case I_TRUNCATE:
			hubo_interrupcion = true;
			break;
		case I_SET:
			// SET (Registro, Valor)
			int retardo = configCpu->RETARDO_INSTRUCCION;
			intervalo_de_pausa(retardo);
			instruccion_set(instruccion[1],instruccion[2]);
			break;
		case I_MOV_IN:{
			// MOV_IN (Registro, Dirección Lógica)
			//instruccion_mov_in(instruccion[1],instruccion[2],pcb);
			int numeroSegmento, offset, tamanioSegmento;

			int dirLogica = atoi(instruccion[1]); // pasa de string a int
			char* registro = strdup(instruccion[2]); // hace el malloc y copia en la varible

			int dirFisica = obtener_direccion_fisica(pcb, dirLogica, &numeroSegmento, &offset, &tamanioSegmento);
			int tamanioALeer = obtener_tamanio_segun_registro(registro);

			if(tamanioALeer + offset <= tamanioSegmento){

				t_parametros_lectura* parametros_a_enviar = malloc(sizeof(t_parametros_lectura));

				parametros_a_enviar->id_proceso = pcb->id_proceso;
				parametros_a_enviar->direccionFisica = dirFisica;
				parametros_a_enviar->tamanio = tamanioALeer;

				enviar_operacion(conexionCpuMemoria, I_MOV_IN, sizeof(t_parametros_lectura), parametros_a_enviar);
				char *valor = recibir_valor_a_escribir(conexionCpuMemoria);
				log_acceso_a_memoria(pcb->id_proceso, "LEER", numeroSegmento, dirFisica, valor,sizeof(valor));
			    instruccion_set(registro, valor);
			    free(valor);
			    free(parametros_a_enviar);
			}else {
				//TODO: EL MOTIVO DE DESALOJO A ENVIAR A KERNEL ES SEGMENTATION FAULT
				loggear_segmentation_fault(pcb->id_proceso, numeroSegmento, offset, tamanioSegmento);
				hubo_interrupcion = true;
			}
			free(registro);
			break;
		}
		case I_MOV_OUT:{
			// MOV_OUT (Dirección Lógica, Registro)
			//instruccion_mov_out(instruccion[1],instruccion[2],pcb);
			int numeroSegmento, offset, tamanioSegmento;

			int dirLogica = atoi(instruccion[1]); // pasa de string a int
			char* registro = strdup(instruccion[2]); // hace el malloc y copia en la varible

			int dirFisica = obtener_direccion_fisica(pcb, dirLogica, &numeroSegmento, &offset, &tamanioSegmento);
			int tamanioALeer = obtener_tamanio_segun_registro(registro);

			if(tamanioALeer + offset <= tamanioSegmento){
				registros_cpu *registrosCPU = pcb->registrosCpu;
				char* valorRegistro = obtener_valor_registro(registro, registrosCPU);
				void* bytesAEnviar = malloc(tamanioALeer);
				memcpy(bytesAEnviar, valorRegistro, tamanioALeer);
				log_acceso_a_memoria(pcb->id_proceso, "ESCRIBIR", numeroSegmento, dirFisica, bytesAEnviar,tamanioALeer);

				t_parametros_escritura* parametros_a_enviar = malloc(sizeof(t_parametros_lectura));

				parametros_a_enviar->id_proceso = pcb->id_proceso;
				parametros_a_enviar->direccionFisica = dirFisica;
				parametros_a_enviar->tamanio = tamanioALeer;
				strcpy((parametros_a_enviar->bytes_a_enviar), ((char*)bytesAEnviar));

				enviar_operacion(conexionCpuMemoria, I_MOV_OUT, sizeof(t_parametros_escritura), parametros_a_enviar);

				if(recibir_operacion(conexionCpuMemoria) != AUX_OK){
					log_error(loggerCpu, "No se pudo recibir la confirmacion de escritura");
					exit(EXIT_FAILURE);
				}

				free(bytesAEnviar);
				free(valorRegistro);
				free(parametros_a_enviar);
			} else{
				//TODO: EL MOTIVO DE DESALOJO A ENVIAR A KERNEL ES SEGMENTATION FAULT
				loggear_segmentation_fault(pcb->id_proceso, numeroSegmento, offset, tamanioSegmento);
				hubo_interrupcion = true;
			}
			break;
		}
		case I_IO:
			// I/O (Tiempo)
		case I_WAIT:
			// WAIT (Recurso)
			hubo_interrupcion = true;
			break;
		case I_SIGNAL:
			// SIGNAL (Recurso)
			hubo_interrupcion = true;
			break;
		case I_CREATE_SEGMENT:
			// CREATE_SEGMENT (Id del Segmento, Tamaño)
			//instruccion_create_segment(instruccion[1],instruccion[2]);
			break;
		case I_DELETE_SEGMENT:
			// DELETE_SEGMENT (Id del Segmento)
			//instruccion_delete_segment(instruccion[1]);
			break;
	}
	return operacion;
}

/************** INSTRUCCIONES ***************************/

void instruccion_set(char* registro,char* valor) {

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

char* recibir_valor_a_escribir(int clienteAceptado){

	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

	recibir_operacion(clienteAceptado);
	buffer = recibir_buffer(&tamanio, clienteAceptado);

	return leer_string(buffer, &desplazamiento);

}

uint32_t obtener_tamanio_segun_registro(char* registro){
    uint32_t tamanio = 0;

    int registro_int = get_int_registro(registro);

    switch(registro_int){
        case REGISTRO_AX:
        case REGISTRO_BX:
        case REGISTRO_CX:
        case REGISTRO_DX:
            tamanio = 4;
            break;
        case REGISTRO_EAX:
        case REGISTRO_EBX:
        case REGISTRO_ECX:
        case REGISTRO_EDX:
            tamanio = 8;
            break;
        case REGISTRO_RAX:
        case REGISTRO_RBX:
        case REGISTRO_RCX:
        case REGISTRO_RDX:
            tamanio = 16;
            break;
        default:
        log_error(loggerCpu,"Registro no valido");
        break;
    }
    return tamanio;
}

char* obtener_valor_registro(char* registro, registros_cpu *registrosCPU){
		char* valor;

		int registro_int = get_int_registro(registro);

		switch(registro_int){
	        case REGISTRO_AX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->AX,4);
	        break;
	        case REGISTRO_BX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->BX,4);
	        break;
	        case REGISTRO_CX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->CX,4);
	        break;
	        case REGISTRO_DX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->DX,4);
	        break;
	        case REGISTRO_EAX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->EAX,8);
	        break;
	        case REGISTRO_EBX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->EBX,8);
	        break;
	        case REGISTRO_ECX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->ECX,8);
	        break;
	        case REGISTRO_EDX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->EDX,8);
	        break;
	        case REGISTRO_RAX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->RAX,16);
	        break;
	        case REGISTRO_RBX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->RBX,16);
	        break;
	        case REGISTRO_RCX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->RCX,16);
	        break;
	        case REGISTRO_RDX:
	        valor = registros_cpu_get_valor_registro(registrosCpu->RDX,16);
	        break;
	        default:
	        log_error(loggerCpu,"Registro no valido");
	        break;
	    }

	    return valor;
}

int get_int_registro(char* registro){
	if (strcmp(registro, "AX") == 0) {
		return REGISTRO_AX;
	} else if (strcmp(registro, "BX") == 0) {
		return REGISTRO_BX;
	} else if (strcmp(registro, "CX") == 0) {
		return REGISTRO_CX;
	} else if (strcmp(registro, "DX") == 0) {
		return REGISTRO_DX;
	} else if (strcmp(registro, "EAX") == 0) {
		return REGISTRO_EAX;
	} else if (strcmp(registro, "EBX") == 0) {
		return REGISTRO_EBX;
	} else if (strcmp(registro, "ECX") == 0) {
		return REGISTRO_ECX;
	} else if (strcmp(registro, "EDX") == 0) {
		return REGISTRO_EDX;
	} else if (strcmp(registro, "RAX") == 0) {
		return REGISTRO_RAX;
	} else if (strcmp(registro, "RBX") == 0) {
		return REGISTRO_RBX;
	} else if (strcmp(registro, "RCX") == 0) {
		return REGISTRO_RCX;
	} else if (strcmp(registro, "RDX") == 0) {
		return REGISTRO_RDX;
	} else {
		return -1;
	}
}

char *registros_cpu_get_valor_registro(char* registro, int tamanioRegistro){
	/*
	if (registro == NULL) {
        char *registroStringAux = malloc((TAMANIO_STRING_VACIO + 1) * sizeof(*registroStringAux));
        registroStringAux[0] = '\0';
        return registroStringAux;
    }
    */

    char *registroString = malloc((tamanioRegistro + 1) * sizeof(*registroString));
    registroString = memcpy(registroString, registro, tamanioRegistro);
    registroString[tamanioRegistro] = '\0';
    return registroString;
}

void log_acceso_a_memoria(uint32_t pid, char* modo, uint32_t idSegmento, uint32_t dirFisica, void* valor, uint32_t tamanio){
    char* valorPrinteable = agregarCaracterNulo(valor, tamanio);
    log_info(loggerCpu, "PID: %d - Acción: %s - Segmento: %d - Dirección Física: %d - Valor: %s", pid, modo, idSegmento, dirFisica, valorPrinteable);
    free(valorPrinteable);
    return;
}
char* agregarCaracterNulo(void* data, uint32_t length){
    char* charData = (char*)data;

    char* str = (char*)malloc((length + 1) * sizeof(char));
    if (str == NULL)
    {
        return NULL;
    }

    // Copiar los caracteres al nuevo char*
    for (uint32_t i = 0; i < length; i++)
    {
        str[i] = charData[i];
    }

    // Agregar el carácter nulo al final
    str[length] = '\0';

    return str;
}

void loggear_segmentation_fault(uint32_t pid, uint32_t numSegmento, uint32_t offset, uint32_t tamanio) {
    log_info(loggerCpu, "PID: %u - Error SEG_FAULT- Segmento: %u - Offset: %u - Tamaño: %u", pid, numSegmento, offset, tamanio);
    return;
}

int obtener_direccion_fisica(PCB* pcb, int dirLogica, int* numero_segmento, int* offset, int* tamanioSegmento) {

    int tam_max_segmento = configCpu->TAM_MAX_SEGMENTO;
    *numero_segmento = dirLogica / tam_max_segmento;
    *offset = dirLogica % tam_max_segmento;
    int base = obtener_base_segmento(pcb, *numero_segmento, tamanioSegmento);
    int direccionFisica = base + *offset;
    return direccionFisica;
}

int obtener_base_segmento(PCB* pcb, int numeroSegmento,  int *tamanio){

    int cantidadSegmentos = list_size(pcb->lista_segmentos);
   	t_segmento* segmentoTabla; // TODO: direccionBase deberia ser int

   	for(int i = 0; i < cantidadSegmentos; i++){
   		segmentoTabla = list_get(pcb->lista_segmentos, i);
   		if(segmentoTabla->id == numeroSegmento){
   			*tamanio = segmentoTabla->size;
   			return *(int*)(segmentoTabla->direccionBase);
   		}
   	}
   	return -1;
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

void* get_registro_cpu(char* registro, registros_cpu* registrosCpu){
	if (strcmp(registro, "AX") == 0) {
		return &(registrosCpu->AX);
	} else if (strcmp(registro, "BX") == 0) {
		return &(registrosCpu->BX);
	} else if (strcmp(registro, "CX") == 0) {
		return &(registrosCpu->CX);
	} else if (strcmp(registro, "DX") == 0) {
		return &(registrosCpu->DX);
	} else if (strcmp(registro, "EAX") == 0) {
		return &(registrosCpu->EAX);
	} else if (strcmp(registro, "EBX") == 0) {
		return &(registrosCpu->EBX);
	} else if (strcmp(registro, "ECX") == 0) {
		return &(registrosCpu->ECX);
	} else if (strcmp(registro, "EDX") == 0) {
		return &(registrosCpu->EDX);
	} else if (strcmp(registro, "RAX") == 0) {
		return &(registrosCpu->RAX);
	} else if (strcmp(registro, "RBX") == 0) {
		return &(registrosCpu->RBX);
	} else if (strcmp(registro, "RCX") == 0) {
		return &(registrosCpu->RCX);
	} else if (strcmp(registro, "RDX") == 0) {
		return &(registrosCpu->RDX);
	}
	return NULL;
}
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
		return NULL;
	}
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
	//agregar_lista_a_paquete(paquete, pcb->lista_segmentos); TODO: no funciona

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
		strtok(palabra, "\n"); // Removemos el salto de linea
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
/*
 * Devuelve el pcb a kernel porque terminó de ejecutar el proceso
 */
void devolver_pcb_kernel(PCB* pcb, int conexion, codigo_operacion codOperacion) {
	enviar_operacion(conexion, codOperacion, sizeof(PCB), pcb);
}
