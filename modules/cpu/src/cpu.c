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

    //enviar_codigo_operacion(conexionCpuMemoria, AUX_SOY_CPU);

    if (conexionCpuMemoria > 0) {
    		enviar_operacion(conexionCpuMemoria, AUX_SOY_CPU, 0, 0);
    }

    int servidorCPU = iniciar_servidor(config, loggerCpu);
    int clienteKernel = esperar_cliente(servidorCPU, loggerCpu);

    pthread_mutex_init(&m_recibir_pcb,NULL);
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
	    //log_warning(loggerCpu, "el codigo recibido en cpu es %d", codDePrueba1);
		PCB* pcb_a_ejecutar = recibir_pcb(clienteKernel);
		//log_warning(loggerCpu, "SE RECIBIO EL PCB DE PID %d EN CPU", pcb_a_ejecutar->id_proceso);
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
	strcpy(registrosCpu->AX, "AX");
    strcpy(registrosCpu->BX, "BX");
    strcpy(registrosCpu->CX, "CX");
    strcpy(registrosCpu->DX, "DX");
    strcpy(registrosCpu->EAX, "EAX");
    strcpy(registrosCpu->EBX, "EBX");
    strcpy(registrosCpu->ECX, "ECX");
    strcpy(registrosCpu->EDX, "EDX");
    strcpy(registrosCpu->RAX, "RAX");
    strcpy(registrosCpu->RBX, "RBX");
    strcpy(registrosCpu->RCX, "RCX");
    strcpy(registrosCpu->RDX, "RDX");

}

void ejecutar_proceso(PCB* pcb, int clienteKernel) {

	cargar_registros(pcb);

	char* instruccion;
	char** instruccion_decodificada;

	//t_list* data_instruccion; // Array para los parametros que necesite una instruccion

	int cantidad_instrucciones = list_size(pcb->lista_instrucciones);
	int posicion_actual = pcb->contador_instrucciones;
	codigo_operacion ultimaOperacion = -1;

    while ((posicion_actual < cantidad_instrucciones) && !hubo_interrupcion) {
	    instruccion = (char *)list_get(pcb->lista_instrucciones, pcb->contador_instrucciones);
		instruccion_decodificada = decode_instruccion(instruccion, loggerCpu);
		if (instruccion_decodificada[0] != NULL) {

			loggear_instruccion(pcb, instruccion_decodificada);

        	ultimaOperacion = ejecutar_instruccion(instruccion_decodificada, pcb);
			if (!hubo_interrupcion) {
				pcb->contador_instrucciones++;
				posicion_actual++;
			}

			//log_info(loggerCpu, "PROGRAM COUNTER: %d", pcb->contador_instrucciones);
		}
    }

    guardar_contexto_de_ejecucion(pcb);

	// Si hubo interrupcion de algun tipo se lo comunico a kernel pero sacamos
	if (hubo_interrupcion) {
		hubo_interrupcion = false;
	}

	//mostrar_pcb(pcb, loggerCpu);
	enviar_pcb(clienteKernel, pcb, ultimaOperacion, loggerCpu);

	// Si tiene que calcular direccion fisica se la mando aparte
	if (ultimaOperacion == I_F_READ || ultimaOperacion == I_F_WRITE) {
			int dirLogica = atoi(instruccion_decodificada[2]); // pasa de string a int
			int tamanio = atoi(instruccion_decodificada[3]);

			int dirFisica = obtener_direcc_fisica(pcb, dirLogica, tamanio);

			log_trace(loggerCpu, "la direcc fisica es: %d", dirFisica);
		// Reescribo la instruccion usando dir fisica en vez de logica
		// long direccionFisica = convertir_dir_logica_a_fisica(pcb, instruccion_decodificada[2]);

		t_paquete* paquete = crear_paquete(AUX_OK);
		// agregar_puntero_a_paquete(paquete, direccionFisica);
		agregar_int_a_paquete(paquete, dirFisica);
		enviar_paquete(paquete, clienteKernel);
		eliminar_paquete(paquete);
		// enviar_operacion(clienteKernel, AUX_OK, sizeof(uintptr_t), direccionFisica);
	}

	free(instruccion);
	free(instruccion_decodificada);
}

void loggear_instruccion(PCB* pcb, char** instruccion){
	char* instruccion_ = malloc(sizeof(char*));
	instruccion_ = strtok(instruccion[0], "\n");

	int operacion = keyFromString(instruccion_);

	switch(operacion){
		case I_F_READ:
		case I_F_WRITE:
			log_info(loggerCpu, "PID: %u - Ejecutando: %s  %s %s %s", pcb->id_proceso, instruccion[0], instruccion[1], instruccion[2], instruccion[3]);
			break;
		case I_SET:
		case I_MOV_IN:
		case I_MOV_OUT:
		case I_TRUNCATE:
		case I_F_SEEK:
		case I_CREATE_SEGMENT:
			log_info(loggerCpu, "PID: %u - Ejecutando: %s  %s %s", pcb->id_proceso, instruccion[0], instruccion[1], instruccion[2]);
			break;
		case I_IO:
		case I_WAIT:
		case I_SIGNAL:
		case I_F_OPEN:
		case I_F_CLOSE:
		case I_DELETE_SEGMENT:
			log_info(loggerCpu, "PID: %u - Ejecutando: %s  %s", pcb->id_proceso, instruccion[0], instruccion[1]);
			break;
		case I_EXIT:
		case I_YIELD:
			log_info(loggerCpu, "PID: %u - Ejecutando: %s", pcb->id_proceso, instruccion[0]);
			break;
	}
}

void cargar_registros(PCB* pcb) { // Acumula basura
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
		case I_IO:
		case I_WAIT:
		case I_SIGNAL:
		case I_CREATE_SEGMENT:
		case I_DELETE_SEGMENT:
			hubo_interrupcion = true;
		break;
		case I_SET: {
			// SET (Registro, Valor)
			int retardo = configCpu->RETARDO_INSTRUCCION;
			intervalo_de_pausa(retardo);
			instruccion_set(instruccion[1],instruccion[2]);
			break;
		}
		case I_MOV_IN:{

			int dirLogica = atoi(instruccion[2]); // pasa de string a int
			//log_warning(loggerCpu, "la direccion logica obtenida por parametro es %d", dirLogica);
			char* registro = strdup(instruccion[1]); // hace el malloc y copia en la varible
			//log_warning(loggerCpu, "el registro obtenido por parametro es %s", registro);

			int tamanio_registro = obtener_tamanio_registro(registro);

			int dirFisica = obtener_direcc_fisica(pcb, dirLogica, tamanio_registro);

			log_trace(loggerCpu, "la direcc fisica es: %d", dirFisica);

			if (dirFisica == -1){
				hubo_interrupcion = true;
				return SEGMENTATION_FAULT;
			}

			t_paquete* paquete = crear_paquete(I_MOV_IN);
			agregar_int_a_paquete(paquete, pcb->id_proceso);
			agregar_int_a_paquete(paquete, dirFisica);
			agregar_int_a_paquete(paquete, tamanio_registro);
			enviar_paquete(paquete, conexionCpuMemoria);
			eliminar_paquete(paquete);


			char *valor = recibir_valor_a_escribir(conexionCpuMemoria);


			log_info(loggerCpu, "PID: %d  -Acción: LEER - Segmento: %d - Dirección Física: %d - Valor: %s", pcb->id_proceso, division_entera(dirLogica, configCpu->TAM_MAX_SEGMENTO), dirFisica, valor);

			instruccion_set(registro, valor);

			free(valor);


			/*
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
			*/
			break;
		}
		case I_MOV_OUT:{
			int dirLogica = atoi(instruccion[1]); // pasa de string a int
			//log_warning(loggerCpu, "la direccion logica obtenida por parametro es %d", dirLogica);
			char* registro = strdup(instruccion[2]); // hace el malloc y copia en la varible
			//log_warning(loggerCpu, "el registro obtenido por parametro es %s", registro);

			int tamanio_registro = obtener_tamanio_registro(registro);

			int dirFisica = obtener_direcc_fisica(pcb, dirLogica, tamanio_registro);
			void* dirFisicaPuntero = (void*) (intptr_t)dirFisica;

			//log_trace(loggerCpu, "la direccion fisica es %d", dirFisica);

			char* valor_registro = obtener_valor_registro(registro, pcb->registrosCpu);

			// char* valor_registro = registros_cpu_get_valor_registro("escribiendoTextoDePrueba", 16);
			int numero_segmento = floor(dirLogica/configCpu->TAM_MAX_SEGMENTO);
			int desplazamiento_segmento=dirLogica % configCpu->TAM_MAX_SEGMENTO;

			if (desplazamiento_segmento + tamanio_registro > obtener_tamanio_segmento(pcb->lista_segmentos, numero_segmento)){
				hubo_interrupcion = true;
				return SEGMENTATION_FAULT;
			}

			t_paquete* paquete = crear_paquete(I_MOV_OUT);
			agregar_int_a_paquete(paquete, pcb->id_proceso);
			// agregar_puntero_a_paquete(paquete, dirFisicaPuntero);
			agregar_int_a_paquete(paquete, dirFisica);
			agregar_int_a_paquete(paquete, tamanio_registro);
			agregar_registro_a_paquete(paquete, valor_registro, tamanio_registro);

			enviar_paquete(paquete, conexionCpuMemoria);
			eliminar_paquete(paquete);

			//log_warning(loggerCpu, "PASA EL ENVIAR OPERACION");

			codigo_operacion cod1Prueba = recibir_operacion(conexionCpuMemoria);
			codigo_operacion cod2Prueba = recibir_operacion(conexionCpuMemoria);

			log_info(loggerCpu, "PID: %d  -Acción: ESCRIBIR - Segmento: %d - Dirección Física: %d - Valor: %s", pcb->id_proceso, division_entera(dirLogica, configCpu->TAM_MAX_SEGMENTO), dirFisica, valor_registro);

			}
			break;
		default:
			log_error(loggerCpu,E__CODIGO_INVALIDO);
		}

	return operacion;
}

int obtener_tamanio_registro(char* registro){

	int codigo = codigo_registro(registro);

	switch(codigo){
		case AX:{return 4;break;}
		case BX:{return 4;break;}
		case CX:{return 4;break;}
		case DX:{return 4;break;}

		case EAX:{return 8;break;}
		case EBX:{return 8;break;}
		case ECX:{return 8;break;}
		case EDX:{return 8;break;}

		case RAX:{return 16;break;}
		case RBX:{return 16;break;}
		case RCX:{return 16;break;}
		case RDX:{return 16;break;}

	}
}

int codigo_registro(char* registro){
	if(strcmp(registro,"AX")==0){
		return AX;
	}else if(strcmp(registro,"BX")==0){
		return BX;
	}else if(strcmp(registro,"CX")==0){
		return CX;
	}else if(strcmp(registro,"DX")==0){
		return DX;
	}else if(strcmp(registro,"EAX")==0){
		return EAX;
	}else if(strcmp(registro,"EBX")==0){
		return EBX;
	}else if(strcmp(registro,"ECX")==0){
		return ECX;
	}else if(strcmp(registro,"EDX")==0){
		return EDX;
	}else if(strcmp(registro,"RAX")==0){
		return RAX;
	}else if(strcmp(registro,"RBX")==0){
		return RBX;
	}else if(strcmp(registro,"RCX")==0){
		return RCX;
	}else if(strcmp(registro,"RDX")==0){
		return RDX;
	}
}
int obtener_direcc_fisica(PCB* pcb, int dirLogica, int tamanio_registro){

	int numero_segmento = floor(dirLogica/configCpu->TAM_MAX_SEGMENTO);
	//log_warning(loggerCpu, "el nro de seg es %d", numero_segmento);
	int desplazamiento_segmento=dirLogica % configCpu->TAM_MAX_SEGMENTO;
	//log_warning(loggerCpu, "el desplazamiento dentro del seg es %d", desplazamiento_segmento);

	if (desplazamiento_segmento + tamanio_registro > obtener_tamanio_segmento(pcb->lista_segmentos, numero_segmento) ){
			log_error(loggerCpu, "PID: %d - Error SEG_FAULT- Segmento: %d - Offset: %d - Tamaño: %d", pcb->id_proceso, numero_segmento, desplazamiento_segmento, tamanio_registro);
			return -1;
	}
	t_segmento* segmento = list_get(pcb->lista_segmentos, numero_segmento);

	//log_warning(loggerCpu, "id segmento %d", segmento->id);
	//log_warning(loggerCpu, "size segmento %d", segmento->tamanio_segmento);
	//log_warning(loggerCpu, "direcc base segmento %p", segmento->direccion_base);

	// long direccion_fisica = (long)(segmento->direccionBase + desplazamiento_segmento);
	return desplazamiento_segmento;
	// return direccion_fisica;
}

int obtener_tamanio_segmento(t_list* lista_segmentos, int numero_segmento){

	for(int i = 0; i < list_size(lista_segmentos); i++){
		t_segmento* segmento = list_get(lista_segmentos, i);
		if(segmento->id == numero_segmento){
			return segmento->size;
		}
	}
	return -1;

}

int division_entera(int operando1, int operando2){
	return (operando1 - (operando1 % operando2)) / operando2;
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

	return leer_registro_de_buffer(buffer, desplazamiento);

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
	//log_warning(loggerCpu, "entra a obtener_direccion_fisica");
    int tam_max_segmento = configCpu->TAM_MAX_SEGMENTO;
    *numero_segmento = dirLogica / tam_max_segmento;
    *offset = dirLogica % tam_max_segmento;
    //log_warning(loggerCpu, "antes de obtener_base_segmento");
    int base = obtener_base_segmento(pcb, *numero_segmento, tamanioSegmento);
    //log_warning(loggerCpu, "despues de obtener_base_segmento");
    int direccionFisica = base + *offset;
    return direccionFisica;
}

int obtener_base_segmento(PCB* pcb, int numeroSegmento,  int *tamanio){

    int cantidadSegmentos = list_size(pcb->lista_segmentos);

   	t_segmento* segmentoTabla; // TODO: direccionBase deberia ser int

   	for(int i = 0; i < cantidadSegmentos; i++){
   		segmentoTabla = list_get(pcb->lista_segmentos, i);
   		//log_warning(loggerCpu, "la base del segmento obtenido es %d", (int)(intptr_t)segmentoTabla->direccionBase);
   		//log_warning(loggerCpu, "el id del segmento obtenido es %d y el id del segmento de la tabla es %d", segmentoTabla->id, numeroSegmento);
   		if(segmentoTabla->id == numeroSegmento){
   			*tamanio = segmentoTabla->size;
   			//log_warning(loggerCpu, "EL TAMAÑO DEL SEGMENTO ES %d", *tamanio);
   			//int base = *(int*)(segmentoTabla->direccionBase);
   			int base = (int)(intptr_t)segmentoTabla->direccionBase;
   			//log_warning(loggerCpu, "LA BASE DEL SEGMENTO ES %d", base);
   			return base;
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


void* convertir_dir_logica_a_fisica(PCB *pcb, char* dirLogicaTexto) {
	uint32_t numeroSegmento, offset, tamanioSegmento;

	char* endptr; // Puntero para manejar errores en la conversión
	uint32_t dirLogica = strtoul(dirLogicaTexto, &endptr, 10); // Convertir la cadena a un valor numérico uint32_t

	// Verificar si hubo algún error en la conversión
	if (*endptr != '\0') {
		log_error(loggerCpu, "El valor no representa un número válido de direccion logica");
	}

	void* dirFisica = obtener_puntero_direccion_fisica(pcb, dirLogica, &numeroSegmento, &offset, &tamanioSegmento);

	log_info(loggerCpu, "Conversion de memoria dirLogica <%s>, dirLogica <%d> a dirFisica <%p>",
		dirLogicaTexto, dirLogica, dirFisica);
    return dirFisica;
}

void* obtener_puntero_direccion_fisica(PCB *pcb,uint32_t dirLogica, uint32_t *numeroSegmento, uint32_t *offset, uint32_t *tamanioSegmento){
    uint32_t tam_max_segmento;
    tam_max_segmento = configCpu->TAM_MAX_SEGMENTO;
    uint32_t numero_de_segmento = (dirLogica / tam_max_segmento);
    *offset = (uint32_t) dirLogica % tam_max_segmento;
    void* base = obtener_base_segmento_puntero(pcb, numero_de_segmento, tamanioSegmento);
    void* direccionFisica = calcular_direccion(base, (size_t)(*offset));
    return direccionFisica;
}

void* obtener_base_segmento_puntero(PCB *pcb, uint32_t numeroSegmento,  uint32_t *tamanio){
    void* base;
    int i = 0;
    uint32_t my_uint32_value;

    int cantidadSegmentos = list_size(pcb->lista_segmentos);
   	t_segmento* segmentoTabla;

   	if (cantidadSegmentos == 0) {
   		log_error(loggerCpu, "Error no hay bases de segmentos en el pcb, fijarse si memoria le dio a kernel la base de segmentos");
   		return 0;
   	}

   	while(i < cantidadSegmentos){
   		segmentoTabla = list_get(pcb->lista_segmentos, i);
   		if(segmentoTabla->id == numeroSegmento){
   			my_uint32_value = (uint32_t) segmentoTabla->direccionBase;
			log_info(loggerCpu, "Base encontrada, en void: %p", segmentoTabla->direccionBase);
			return segmentoTabla->direccionBase;
   		}
   		i++;
   	}
	log_error(loggerCpu, "DIR LOGICA NO encontrada a partir de nro de segmento <%d>", numeroSegmento);
    return NULL;
}
