#include "../include/cpu.h"
#include "../../kernel/src/pcb.c"
#include "../../shared/src/funciones.c"
#include "../../shared/src/funcionesServidor.c"
#include "../../shared/src/funcionesCliente.c"

int main(int argc, char** argv)
{
    t_log* logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);
    cargar_config(config);

    int servidorCPU = iniciar_servidor(config, logger);


    int conexionMemoria = armar_conexion(config, MEMORIA, logger);
    /*
    handshakeConMemoria(conexionMemoria);
    // int clienteAceptado = esperar_cliente(servidorCpu, logger);

    cpu_registers registros;
    inicializar_registros(registros);
   

    //pthread_t hilo_ejecucion;
    pthread_t hilo_dispatcher, hilo_interrupcion;

	//pthread_create(&hilo_ejecucion, NULL, (void *) procesar_instruccion, int servidorCPU); //  TODO: Avisar posibles errores o si el kernel se desconecto.
    pthread_create(&hilo_dispatcher, NULL, (void *) procesar_dispatcher, NULL); 
    pthread_create(&hilo_interrupcion, NULL, (void *) procesar_interrupcion, NULL);
    pthread_join(hilo_dispatcher, NULL);
    pthread_join(hilo_interrupcion, NULL);
    */
    return 0;
    //terminar_programa(servidorCpu, logger, config);
}

void cargar_config(t_config* config)
{
	cpuConfig->RETARDO_INSTRUCCION = config_get_string_value(config, "RETARDO_INSTRUCCION");
	cpuConfig->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	cpuConfig->PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	cpuConfig->PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	cpuConfig->TAM_MAX_SEGMENTO = config_get_int_value(config, "TAM_MAX_SEGMENTO");
}

/*

void handshakeConMemoria(int socket){
	send(socket, "CPU", 3, 0);
	char* respuesta_de_memoria = receive(socket);
	if (strcmp(respuesta_de_memoria, "MEMORIA") == 0) {
	    printf("Handshake exitoso.\n");
	} else {
	    printf("Error en el handshake.\n");
	}
	free(respuesta_de_memoria);
}

void inicializar_registros(registros* registros)
{
	registros->AX = 0;
	registros->BX = 0;
	registros->CX = 0;
	registros->DX = 0;
	registros->EAX = 0;
	registros->EBX = 0;
	registros->ECX = 0;
	registros->EDX = 0;
	registros->RAX = 0;
	registros->RBX = 0;
	registros->RCX = 0;
	registros->RDX = 0;
}

void procesar_instruccion(int servidorCPU)
{
	int socket_kernel = accept(servidorCPU, NULL, NULL);
	PCB* pcb;
	pcb = recibir_pcb(servidorCPU, logger);
	ejecutar_proceso(pcb);
}

PCB* recibir_pcb(int servidor, t_log* logger)
{
	PCB* pcb = malloc(sizeof(PCB));
	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

	buffer = recibir_buffer(&tamanio, servidor);

	pcb->id_proceso = leer_int(buffer, &desplazamiento);
	pcb->lista_instrucciones = leer_string_array(buffer, &desplazamiento);
	pcb->program_counter = leer_int(buffer, &desplazamiento);
	pcb->cpu_register->[AX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[BX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[CX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[DX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[EAX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[EBX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[ECX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[EDX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[RAX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[RBX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[RCX] = leer_string(buffer, &desplazamiento);
	pcb->cpu_register->[RDX] = leer_string(buffer, &desplazamiento);

	pcb->lista_segmentos = list_create();
	int cantidad_de_segmentos = leer_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_segmentos; i++) {
	    t_segmento* segmento = malloc(sizeof(t_segmento));

	    segmento->ID = leer_int(buffer, &desplazamiento);
	    //segmento->direccion_base = leer_int(buffer, &desplazamiento);
	    segmento->tamanio = leer_int(buffer, &desplazamiento);

	    list_add(pcb->lista_segmentos, segmento);
	}

	pcb->processor_burst = read_float(buffer, &desplazamiento);
	pcb->ready_timestamp = leer_int(buffer, &desplazamiento);

	pcb->lista_archivos_abiertos = list_create();
	int cantidad_de_archivos = leer_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_archivos; i++) {
			archivo_abierto_t* archivo_abierto = malloc(sizeof(archivo_abierto_t));

		    archivo_abierto->ID = leer_int(buffer, &desplazamiento);
		    archivo_abierto->posicion_puntero = leer_int(buffer, &desplazamiento);

		    list_add(pcb->lista_archivos_abiertos, archivo_abierto);
		}

	return pcb;
}

void ejecutar_proceso(PCB* pcb)
{

	set_registros(pcb);

	char* instruccion = malloc(sizeof(char*));
	char** instruccion_decodificada = malloc(sizeof(char*));

	//int cantidad_instrucciones = list_size(pcb->lista_instrucciones);
	//int posicion_actual = 0;
    
    // Se tienen en cuenta las posibles causas de interrupciones al proceso
    // 1. Termina el proceso (f_eop)
    // 2. Se produce una interrupcion (f_interruption)
    // 3. IO Exception (f_io)
    // 4. Page Fault Exception (f_pagefault)
    // 5. Segmentation Fault (f_segfault)
    // Denotamos con f_<var> (f: Flag)
	
    //while (posicion_actual < cantidad_instrucciones){
	while(f_eop!=1 && f_interruption!=1 && f_io!=1 && f_pagefault!=1 && f_segfault!=1){
        instruccion = string_duplicate(fetch_siguiente_instruccion(pcb));
		instruccion_decodificada = decode_instruccion(instruccion);
        
        log_info(logger, "Ejecutando instruccion: %s", instruccion_decodificada[0];
        ejecutar_instruccion(instruccion_decodificada, pcb);

        // Evaluar si la instruccion genero una excepcion "f_pagefault"
        // Caso afirmativo => No se actualiza el program counter del pcb
        if(f_pagefault!=1){ actualizar_program_counter(pcb); }

        log_info(logger, "PROGRAM COUNTER: %d", pcb->program_counter);

        usleep(atoi(cpu->RETARDO_INSTRUCCION)*1000);
        log_info(logger, "Se suspendio el proceso por retardo de la instruccion...", ENTER);
	}

    log_info(logger, "Se salio de la ejecucion. Guardando el contexto de ejecucion...", ENTER);
	guardar_contexto_de_ejecucion(pcb);

    if(f_eop){
        f_eop = 0;
        f_interruption = 0;
    }    
}

void actualizar_program_counter(pcb)
{
    pcb->program_counter++;
	posicion_actual++;
}

void set_registros(PCB* pcb)
{
	registrosCPU->AX = pcb->cpu_register->AX;
	registrosCPU->BX = pcb->cpu_register->BX;
	registrosCPU->CX = pcb->cpu_register->CX;
	registrosCPU->DX = pcb->cpu_register->DX;
	registrosCPU->AX = pcb->cpu_register->EAX;
	registrosCPU->BX = pcb->cpu_register->EBX;
	registrosCPU->CX = pcb->cpu_register->ECX;
	registrosCPU->DX = pcb->cpu_register->EDX;
	registrosCPU->AX = pcb->cpu_register->RAX;
	registrosCPU->BX = pcb->cpu_register->RBX;
	registrosCPU->CX = pcb->cpu_register->RCX;
	registrosCPU->DX = pcb->cpu_register->RDX;
}

void ejecutar_instruccion(char** instruccion_decodificada, PCB* pcb)
{

	 switch(instruccion_decodificada[0]){
	 	 case "SET":
	 		 asignar_valor_a_registro(instruccion_decodificada[1],instruccion_decodificada[2]);
	 		 break;
	 	 case "YIELD":
	 		 break;
	 	 case "EXIT":
	 		send_pcb_package(socket_kernel, pcb);
	 		break;
     }
}

void asignar_valor_a_registro(char* registro,char* valor)
{

	int set_valor = atoi(valor);

	if (strcmp(registro, "AX") == 0) {
		registrosCPU->AX = set_valor;
	} else if (strcmp(registro, "BX") == 0) {
		registrosCPU->BX = set_valor;
	} else if (strcmp(registro, "CX") == 0) {
		registrosCPU->CX = set_valor;
	} else if (strcmp(registro, "DX") == 0) {
		registrosCPU->DX = set_valor;
	} else if (strcmp(registro, "EAX") == 0) {
		registrosCPU->EAX = set_valor;
	} else if (strcmp(registro, "EBX") == 0) {
		registrosCPU->EBX = set_valor;
	} else if (strcmp(registro, "ECX") == 0) {
		registrosCPU->ECX = set_valor;
	} else if (strcmp(registro, "EDX") == 0) {
		registrosCPU->EDX = set_valor;
	} else if (strcmp(registro, "RAX") == 0) {
		registrosCPU->RAX = set_valor;
	} else if (strcmp(registro, "RBX") == 0) {
		registrosCPU->RBX = set_valor;
	} else if (strcmp(registro, "RCX") == 0) {
		registrosCPU->RCX = set_valor;
	} else if (strcmp(registro, "RDX") == 0) {
		registrosCPU->RDX = set_valor;
	}
}

char* fetch_siguiente_instruccion(PCB* pcb)
{
	return pcb->lista_instrucciones[pcb->program_counter];
}

char** decode_instruccion(char* linea_a_parsear)
{
	char** instruccion = string_split(linea_a_parsear, " ");
	if(instruccion[0] == NULL){
	    log_info(logger, "Hay linea vacía.");
	}
	return instruccion;
}

void guardar_contexto_de_ejecucion(PCB* pcb)
{
	cpu_register->AX = pcb->registrosCPU->AX;
	cpu_register->BX = pcb->registrosCPU->BX;
	cpu_register->CX = pcb->registrosCPU->CX;
	cpu_register->DX = pcb->registrosCPU->DX;
	cpu_register->EAX = pcb->registrosCPU->EAX;
	cpu_register->EBX = pcb->registrosCPU->EBX;
	cpu_register->ECX = pcb->registrosCPU->ECX;
	cpu_register->EDX = pcb->registrosCPU->EDX;
	cpu_register->RAX = pcb->registrosCPU->RAX;
	cpu_register->RBX = pcb->registrosCPU->RBX;
	cpu_register->RCX = pcb->registrosCPU->RCX;
	cpu_register->RDX = pcb->registrosCPU->RDX;
}
*/


