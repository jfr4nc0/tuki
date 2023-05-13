#include "../include/cpu.h"

int main(int argc, char** argv)
{
    t_log* logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);
    cargar_config(config);

    int servidorCpu = iniciar_servidor(config, logger);

    // Conexion con memoria
    int conexionMemoria = armar_conexion(config, MEMORIA, logger);
    handshakeConMemoria(conexionMemoria);
    // int clienteAceptado = esperar_cliente(servidorCpu, logger);

    registros* registros;
    inicializar_registros(registros);

    pthread_t hilo_ejecucion;
	pthread_create(&hilo_ejecucion, NULL, (void *) ejecutar_instruccion, NULL); //  TODO: Avisar posibles errores o si el kernel se desconecto.

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

void ejecutar_instruccion(){
	PCB* pcb;
	pcb = recibir_pcb_de_kernel(servidorCpu, logger);
	ejecutar_proceso(pcb);

}

void recibir_pcb_de_kernel(int servidor, t_log* logger){
	// TODO
}

void ejecutar_proceso(PCB* pcb){
	// TODO: desarrollar + llamar a ejecutar_instruccion();
}

void ejecutar_instruccion(){
	/*
	 switch(?){
	 	 case SET:

	 	 	 break;
	 	 case YIELD:

	 	 	 break;
	 	 case EXIT:

	 	 	 break;
	 }
	 */
}

// TODO: Etapas fetch y decode.
char* fetch(){

}

char** decode(){

}
