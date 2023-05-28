#include "../include/cpu.h"
#include "../../shared/src/funciones.c"
#include "../../shared/src/funcionesServidor.c"
#include "../../shared/src/funcionesCliente.c"

t_log* loggerCpu;
cpu_config_t* configCpu;

int conexionCpuKernel;
registros_cpu* registrosCpu;

bool exitInstruccion, desalojoOpcional = false;

int main(int argc, char** argv) {
    t_log* loggerCpu = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, loggerCpu);
    cargar_config(config);

    int servidorCPU = iniciar_servidor(config, loggerCpu);


    int conexionCpuMemoria = armar_conexion(config, MEMORIA, loggerCpu);
	conexionCpuKernel = armar_conexion(config, KERNEL, loggerCpu);

    inicializar_registros();


    //pthread_t hilo_ejecucion;
    pthread_t hilo_dispatcher;

	//pthread_create(&hilo_ejecucion, NULL, (void *) procesar_instruccion, int servidorCPU); //  TODO: Avisar posibles errores o si el kernel se desconecto.
    pthread_create(&hilo_dispatcher, NULL, (void *) procesar_instruccion, &servidorCPU);

    pthread_join(hilo_dispatcher, NULL);

    terminar_programa(conexionCpuKernel, loggerCpu, config);

    liberar_conexion(conexionCpuMemoria);
    free(registrosCpu);
    free(configCpu);

    return 0;
}

void cargar_config(t_config* config) {
	configCpu = malloc(sizeof(cpu_config_t));
	configCpu->RETARDO_INSTRUCCION = config_get_string_value(config, "RETARDO_INSTRUCCION");
	configCpu->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	configCpu->PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	configCpu->PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	configCpu->TAM_MAX_SEGMENTO = config_get_int_value(config, "TAM_MAX_SEGMENTO");
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

void* procesar_instruccion(int servidorCPU) {
	int clienteAceptado = esperar_cliente(servidorCPU, loggerCpu);
	PCB* pcb;
	pcb = recibir_pcb(clienteAceptado);
	ejecutar_proceso(pcb);
	free(pcb);

	return NULL;
}

PCB* recibir_pcb(int clienteAceptado) {
	PCB* pcb = malloc(sizeof(PCB));
	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

	buffer = recibir_buffer(&tamanio, clienteAceptado);

	pcb->id_proceso = leer_int(buffer, &desplazamiento);
	pcb->lista_instrucciones = leer_string_array(buffer, &desplazamiento);
	pcb->contador_instrucciones = leer_int(buffer, &desplazamiento);
	pcb->registrosCpu->AX = leer_int(buffer, &desplazamiento);
	pcb->registrosCpu->BX = leer_int(buffer, &desplazamiento);
	pcb->registrosCpu->CX = leer_int(buffer, &desplazamiento);
	pcb->registrosCpu->DX = leer_int(buffer, &desplazamiento);
	pcb->registrosCpu->EAX = leer_long(buffer, &desplazamiento);
	pcb->registrosCpu->EBX = leer_long(buffer, &desplazamiento);
	pcb->registrosCpu->ECX = leer_long(buffer, &desplazamiento);
	pcb->registrosCpu->EDX = leer_long(buffer, &desplazamiento);
	pcb->registrosCpu->RAX = leer_long_long(buffer, &desplazamiento);
	pcb->registrosCpu->RBX = leer_long_long(buffer, &desplazamiento);
	pcb->registrosCpu->RCX = leer_long_long(buffer, &desplazamiento);
	pcb->registrosCpu->RDX = leer_long_long(buffer, &desplazamiento);

	pcb->lista_segmentos = list_create();
	int cantidad_de_segmentos = leer_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_segmentos; i++) {
	    t_segmento* segmento = malloc(sizeof(t_segmento));

	    segmento->id = leer_int(buffer, &desplazamiento);
	    //segmento->direccion_base = leer_int(buffer, &desplazamiento);
	    segmento->tamanio = leer_int(buffer, &desplazamiento);

	    list_add(pcb->lista_segmentos, segmento);

	    free(segmento);
	}

	pcb->processor_burst = leer_float(buffer, &desplazamiento);
	pcb->ready_timestamp = leer_int(buffer, &desplazamiento);

	pcb->lista_archivos_abiertos = list_create();
	int cantidad_de_archivos = leer_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_archivos; i++) {
			archivo_abierto_t* archivo_abierto = malloc(sizeof(archivo_abierto_t));

		    archivo_abierto->id = leer_int(buffer, &desplazamiento);
		    archivo_abierto->posicion_puntero = leer_int(buffer, &desplazamiento);

		    list_add(pcb->lista_archivos_abiertos, archivo_abierto);
		    free(archivo_abierto);
		}

	return pcb;
}

void ejecutar_proceso(PCB* pcb) {

	set_registros(pcb);

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

	// while(f_eop!=1 && f_interruption!=1 && f_io!=1 && f_pagefault!=1 && f_segfault!=1){

    while ((posicion_actual < cantidad_instrucciones) && !exitInstruccion && !desalojoOpcional) {
	    instruccion = string_duplicate((char *)list_get(pcb->lista_instrucciones, pcb->contador_instrucciones));
		instruccion_decodificada = decode_instruccion(instruccion);

        log_info(loggerCpu, "Ejecutando instruccion: %s", instruccion_decodificada[0]);
        ejecutar_instruccion(instruccion_decodificada, pcb);

        // Evaluar si la instruccion genero una excepcion "f_pagefault"
        // Caso afirmativo => No se actualiza el program counter del pcb
        pcb->contador_instrucciones++;
		posicion_actual++;

        log_info(loggerCpu, "PROGRAM COUNTER: %d", pcb->contador_instrucciones);

        usleep(atoi(configCpu->RETARDO_INSTRUCCION)*1000);
        log_info(loggerCpu, "Se suspendio el proceso por retardo de la instruccion...");
    }
    free(instruccion);
    free(instruccion_decodificada);

    log_info(loggerCpu, "Se salió de la ejecucion en la instrucción %s. Guardando el contexto de ejecucion...", instruccion);
	guardar_contexto_de_ejecucion(pcb);

	if (exitInstruccion) {
		exitInstruccion = false;
		notificar_instruccion(pcb, conexionCpuKernel, OP_EXIT);
	} else if(desalojoOpcional) {
		desalojoOpcional = false;
		notificar_instruccion(pcb, conexionCpuKernel, OP_YIELD);
	}
}

void set_registros(PCB* pcb) {
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

void ejecutar_instruccion(char** instruccion_decodificada, PCB* pcb) {
	char* comandoInstruccion = instruccion_decodificada[0];

    if (comandoInstruccion) {
		if(strcmp(comandoInstruccion, "SET")) {
			set_registro(instruccion_decodificada[1],instruccion_decodificada[2]);
		} else if(strcmp(comandoInstruccion, "YIELD")) {
			desalojoOpcional = true;
		} else if(strcmp(comandoInstruccion, "EXIT")) {
			exitInstruccion = true;
		}
		log_debug(loggerCpu, cantidad_strings_a_mostrar(2),"Instruccion ejecutada: ", comandoInstruccion);
	}
}

void set_registro(char* registro,char* valor) {

	int set_valor = atoi(valor);

	if (strcmp(registro, "AX") == 0) {
		registrosCpu->AX = set_valor;
	} else if (strcmp(registro, "BX") == 0) {
		registrosCpu->BX = set_valor;
	} else if (strcmp(registro, "CX") == 0) {
		registrosCpu->CX = set_valor;
	} else if (strcmp(registro, "DX") == 0) {
		registrosCpu->DX = set_valor;
	} else if (strcmp(registro, "EAX") == 0) {
		registrosCpu->EAX = set_valor;
	} else if (strcmp(registro, "EBX") == 0) {
		registrosCpu->EBX = set_valor;
	} else if (strcmp(registro, "ECX") == 0) {
		registrosCpu->ECX = set_valor;
	} else if (strcmp(registro, "EDX") == 0) {
		registrosCpu->EDX = set_valor;
	} else if (strcmp(registro, "RAX") == 0) {
		registrosCpu->RAX = set_valor;
	} else if (strcmp(registro, "RBX") == 0) {
		registrosCpu->RBX = set_valor;
	} else if (strcmp(registro, "RCX") == 0) {
		registrosCpu->RCX = set_valor;
	} else if (strcmp(registro, "RDX") == 0) {
		registrosCpu->RDX = set_valor;
	}

}

char** decode_instruccion(char* linea_a_parsear) {
	char** instruccion = string_split(linea_a_parsear, " ");
	if(instruccion[0] == NULL){
	    log_info(loggerCpu, "Se ignora linea vacía.");
	}
	return instruccion;
}

void guardar_contexto_de_ejecucion(PCB* pcb) {
	registrosCpu->AX = pcb->registrosCpu->AX;
	registrosCpu->BX = pcb->registrosCpu->BX;
	registrosCpu->CX = pcb->registrosCpu->CX;
	registrosCpu->DX = pcb->registrosCpu->DX;
	registrosCpu->EAX = pcb->registrosCpu->EAX;
	registrosCpu->EBX = pcb->registrosCpu->EBX;
	registrosCpu->ECX = pcb->registrosCpu->ECX;
	registrosCpu->EDX = pcb->registrosCpu->EDX;
	registrosCpu->RAX = pcb->registrosCpu->RAX;
	registrosCpu->RBX = pcb->registrosCpu->RBX;
	registrosCpu->RCX = pcb->registrosCpu->RCX;
	registrosCpu->RDX = pcb->registrosCpu->RDX;
}
