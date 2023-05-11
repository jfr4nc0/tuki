#include "../include/cpu.h"
#include "utils.c"
#include "../../kernel/include/pcb.h"

int main(int argc, char** argv)
{
    t_log* logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);
    cargar_config(config);

    int servidorCPU = iniciar_servidor(config, logger);

    // Conexion con memoria
    int conexionMemoria = armar_conexion(config, MEMORIA, logger);
    handshakeConMemoria(conexionMemoria);
    // int clienteAceptado = esperar_cliente(servidorCpu, logger);

    cpu_register_t registros;
    inicializar_registros(registros);

    pthread_t hilo_ejecucion;
	pthread_create(&hilo_ejecucion, NULL, (void *) procesar_instruccion, int servidorCPU); //  TODO: Avisar posibles errores o si el kernel se desconecto.

    //terminar_programa(servidorCpu, logger, config);
}

void cargar_config(t_config* config){
	cpu_config->RETARDO_INSTRUCCION = config_get_string_value(config, "RETARDO_INSTRUCCION");
	cpu_config->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	cpu_config->PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	cpu_config->PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	cpu_config->TAM_MAX_SEGMENTO = config_get_int_value(config, "TAM_MAX_SEGMENTO");
}

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

void inicializar_registros(registros* registros){
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

void procesar_instruccion(int servidorCPU){
	int socket_kernel = accept(servidorCPU, NULL, NULL);
	PCB* pcb;
	pcb = recibir_pcb_de_kernel(servidorCPU, logger);
	ejecutar_proceso(pcb);

}

PCB* recibir_pcb_de_kernel(int servidor, t_log* logger){
	PCB* pcb = malloc(sizeof(PCB));
	char* buffer;
	int tamanio = 0;
	int desplazamiento = 0;

	buffer = recibir_buffer(&tamanio, servidor);

	pcb->pid = read_int(buffer, &desplazamiento);
	pcb->lista_instrucciones = read_string_array(buffer, &desplazamiento);
	pcb->program_counter = read_int(buffer, &desplazamiento);
	pcb->cpu_register->[AX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[BX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[CX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[DX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[EAX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[EBX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[ECX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[EDX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[RAX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[RBX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[RCX] = read_string(buffer, &desplazamiento);
	pcb->cpu_register->[RDX] = read_string(buffer, &desplazamiento);

	pcb->lista_segmentos = list_create();
	int cantidad_de_segmentos = read_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_segmentos; i++) {
	    t_segmento* segmento = malloc(sizeof(t_segmento));

	    segmento->ID = read_int(buffer, &desplazamiento);
	    //segmento->direccion_base = read_int(buffer, &desplazamiento);
	    segmento->tamanio = read_int(buffer, &desplazamiento);

	    list_add(pcb->lista_segmentos, segmento);
	}

	pcb->processor_burst = read_float(buffer, &desplazamiento);
	pcb->ready_timestamp = read_int(buffer, &desplazamiento);

	pcb->lista_archivos_abiertos = list_create();
	int cantidad_de_archivos = read_int(buffer, &desplazamiento);
	for (int i = 0; i < cantidad_de_archivos; i++) {
			archivo_abierto_t* archivo_abierto = malloc(sizeof(archivo_abierto_t));

		    archivo_abierto->ID = read_int(buffer, &desplazamiento);
		    archivo_abierto->posicion_puntero = read_int(buffer, &desplazamiento);

		    list_add(pcb->lista_archivos_abiertos, archivo_abierto);
		}

	return pcb;
}

void ejecutar_proceso(PCB* pcb){

	set_registros(pcb);

	char* instruccion = malloc(sizeof(char*));
	char** instruccion_decodificada = malloc(sizeof(char*));

	int cantidad_instrucciones = list_size(pcb->lista_instrucciones);
	int posicion_actual = 0;
	while (posicion_actual < cantidad_instrucciones){
		instruccion = string_duplicate(fetch_siguiente_instruccion(pcb));
		instruccion_decodificada = decode_instruccion(instruccion);
		ejecutar_instruccion(instruccion_decodificada, pcb);
		pcb->program_counter++;
		posicion_actual++;
	}
	guardar_contexto_de_ejecucion(pcb);
}

void set_registros(PCB* pcb){
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

void ejecutar_instruccion(char** instruccion_decodificada, PCB* pcb){

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

void asignar_valor_a_registro(char* registro,char* valor){

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

char* fetch_siguiente_instruccion(PCB* pcb){
	return pcb->lista_instrucciones[pcb->program_counter];
}

char** decode_instruccion(char* linea_a_parsear){
	char** instruccion = string_split(linea_a_parsear, " ");
	if(instruccion[0] == NULL){
	    log_info(logger, "Hay linea vacía.");
	}
	return instruccion;
}

void guardar_contexto_de_ejecucion(PCB* pcb){
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
