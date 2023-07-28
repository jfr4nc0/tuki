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
	// Kernel no debería mandar dos pcbs simultaneamente a cpu, por las dudas tener en cuenta igual
	PCB* pcb_a_ejecutar = recibir_pcb(clienteKernel);

	ejecutar_proceso(pcb_a_ejecutar, clienteKernel);

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

	pcb->estimacion_rafaga = leer_double(buffer, &desplazamiento);
	pcb->ready_timestamp = leer_double(buffer, &desplazamiento);

	return pcb;
}

void ejecutar_proceso(PCB* pcb, int clienteKernel) {

	cargar_registros(pcb);

	// ¿Por que se le hace malloc?
	char* instruccion = malloc(sizeof(char*));
	char** instruccion_decodificada = malloc(sizeof(char*));

	t_list* data_instruccion; // Array para los parametros que necesite una instruccion

	int cantidad_instrucciones = list_size(pcb->lista_instrucciones);
	int posicion_actual = 0;
	codigo_operacion ultimaOperacion = -1;

    while ((posicion_actual < cantidad_instrucciones) && !hubo_interrupcion) {
	    instruccion = string_duplicate((char *)list_get(pcb->lista_instrucciones, pcb->contador_instrucciones));
		instruccion_decodificada = decode_instruccion(instruccion, loggerCpu);

        log_info(loggerCpu, "PID: <%u> - Ejecutando: %s", pcb->id_proceso, instruccion_decodificada[0]);
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

	int operacion = keyFromString(instruccion[0]);

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
			//instruccion_mov_in(instruccion[1],instruccion[2],pcb);
			uint32_t numeroSegmento, offset, tamanioSegmento;
			char* registro_;
			strcpy(registro_, instruccion[1]);
			uint32_t dirLogica = instruccion[2];
			uint32_t dirFisica = obtener_direccion_fisica(pcb, dirLogica, &numeroSegmento, &offset, &tamanioSegmento);
			uint32_t tamanioALeer = obtener_tamanio_segun_registro(registro);


			if((tamanioALeer + offset) <= tamanioSegmento){

				t_parametros_lectura* parametros_a_enviar;

				parametros_a_enviar->id_proceso = pcb->id_proceso;
				parametros_a_enviar->direccionFisica = dirFisica;
				parametros_a_enviar->tamanio = tamanioALeer;

				enviar_operacion(conexionMemoria, I_MOV_IN, sizeof(t_parametros_lectura), parametros_a_enviar);
				char *valor = recibir_valor_a_escribir(conexionMemoria);
				log_acceso_a_memoria(pcb->id_proceso, "LEER", numeroSegmento, dirFisica, valor,sizeof(valor));
			    instruccion_set(registro_, valor);
			    free(valor);
			    free(parametros_a_enviar);
			}else {
				//TODO: EL MOTIVO DE DESALOJO ES SEGMENTATION FAULT
				hubo_interrupcion = true;
			}
			break;
		case I_MOV_OUT:
			// MOV_OUT (Dirección Lógica, Registro)
			//instruccion_mov_out(instruccion[1],instruccion[2],pcb);
			uint32_t numeroSegmento, offset, tamanioSegmento;
			char* registro;
			strcpy(registro, instruccion[1]);
			uint32_t dirLogica = instruccion[2];
			uint32_t dirFisica = obtener_direccion_fisica(pcb, dirLogica, &numeroSegmento, &offset, &tamanioSegmento);
			uint32_t tamanioALeer = obtener_tamanio_segun_registro(registro);

			if((tamanioALeer + offset) <= tamanioSegmento){
				registros_cpu *registrosCPU = pcb->registrosCpu;
				char* valorRegistro = obtener_valor_registro(registro, registrosCPU);
				void* bytesAEnviar = malloc(tamanioALeer);
				memcpy(bytesAEnviar, valorRegistro, tamanioALeer);
				log_acceso_a_memoria(pcb->id_proceso, "ESCRIBIR", numeroSegmento, dirFisica, bytesAEnviar,tamanioALeer);

				t_parametros_escritura* parametros_a_enviar;
				parametros_a_enviar->id_proceso = pcb->id_proceso;
				parametros_a_enviar->direccionFisica = dirFisica;
				parametros_a_enviar->tamanio = tamanioALeer;
				strcpy(parametros_a_enviar->bytes_a_enviar, (char*)bytesAEnviar, sizeof(bytesAEnviar));

				enviar_operacion(conexionMemoria, I_MOV_OUT, sizeof(t_parametros_escritura), parametros_a_enviar);

				if(recibir_operacion(conexionMemoria) != AUX_OK){
					log_error(loggerCpu, "No se pudo recibir la confirmacion de escritura");
					exit(EXIT_FAILURE);
				}

				free(bytesAEnviar);
				free(valorRegistro);
				free(parametros_a_enviar);
			} else{
				//TODO: EL MOTIVO DE DESALOJO ES SEGMENTATION FAULT
				hubo_interrupcion = true;
			}
			break;
		case I_IO:
			// I/O (Tiempo)
			hubo_interrupcion = true;
			break;
		case I_F_OPEN:
			// F_OPEN (Nombre Archivo)
			hubo_interrupcion = true;
			break;
		case I_F_CLOSE:
			// F_CLOSE (Nombre Archivo)
			hubo_interrupcion = true;
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
			hubo_interrupcion = true;
			break;
		case I_SIGNAL:
			// SIGNAL (Recurso)
			hubo_interrupcion = true;
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

	return leer_string(buffer, desplazamiento);

}

uint32_t obtener_tamanio_segun_registro(char* registro){
    uint32_t tamanio = 0;

    switch(registro){
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
	    switch(registro){
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

void log_acceso_a_memoria(uint32_t pid, char* modo, uint32_t idSegmento, uint32_t dirFisica, void* valor, uint32_t tamanio)
{
    char* valorPrinteable = agregarCaracterNulo(valor, tamanio);
    log_info(cpuLogger, "PID: <%d> - Acción: <%s> - Segmento: <%d> - Dirección Física: <%d> - Valor: <%s>", pid, modo, idSegmento, dirFisica, valorPrinteable);
    free(valorPrinteable);
    return;
}

uint32_t obtener_direccion_fisica(PCB *pcb,uint32_t dirLogica, uint32_t *numeroSegmento, uint32_t *offset, uint32_t *tamanioSegmento){
    *numeroSegmento = __obtener_numero_segmento(dirLogica);

    uint32_t tam_max_segmento;
    tam_max_segmento = configCpu->TAM_MAX_SEGMENTO;
    uint32_t numero_de_segmento = (dirLogica / tam_max_segmento);
    *offset = (uint32_t) dirLogica % tam_max_segmento;
    uint32_t base = obtener_base_segmento(pcb, *numeroSegmento, tamanioSegmento);
    uint32_t direccionFisica = base + *offset;
    return direccionFisica;
}

uint32_t obtener_base_segmento(PCB *pcb, uint32_t numeroSegmento,  uint32_t *tamanio){
    uint32_t base;
    uint32_t i = 0;

    int cantidadSegmentos = list_size(pcb->lista_segmentos);
   	t_segmento* segmentoTabla;

   	while(i < cantidadSegmentos){
   		segmentoTabla = list_get(pcb->lista_segmentos, i);
   		if(segmentoTabla->id == numeroSegmento){
   			base = (uint32_t)(segmentoTabla->direccionBase);
   			*tamanio = segmentoTabla->size;
   		}
   		i++;
   	}
    return base;
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

int get_dir_fisica(t_segmento* segmento ,char* dir_logica, int segment_max){
	/*	Esquema de memoria: Segmentacion
	 * 	Direccion Logica: [ Nro Segmento | direccionBase ]
	 *	@return: La direccion fisica
	 */
	segmento->id = floor(atoi(dir_logica)/segment_max);
	segmento->direccionBase = atoi(dir_logica)%segment_max;
	segmento->size = segmento->id + segmento->direccionBase;

	if(segmento->size > segment_max){
		return -1;
	} else { return segmento->size; }
}
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
	} else {
		return;
	}
}

/*
void instruccion_mov_in(char* registro, char* dir_logica, PCB* pcb) {
	/*
	 * Lee el valor de memoria corresponfiente a la Direccion Logica y lo almacena en el registro
	 */
/*	t_segmento* segmento;
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
}
*/
void instruccion_mov_out(char* dir_logica,char* registro, PCB* pcb) {
	/*
	 * Lee el valor del Registro y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica.
	 */
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
	agregar_lista_a_paquete(paquete, pcb->lista_archivos_abiertos);
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
void instruccion_f_seek(char* nombre_archivo, char* posicion) {

}
void instruccion_f_read(char* nombre_archivo, char* dir_logica, char* cant_bytes) {

}
void instruccion_f_write(char* nombre_archivo, char* dir_logica, char* cant_bytes) {

}
void instruccion_f_truncate(char* nombre_archivo,char* tamanio) {

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
