#include "../include/kernel.h"
#include "../../shared/src/funciones.c"
#include "../../shared/shared.h"
#include "../../cpu/src/cpu.c"

int main(int argc, char** argv) {
    logger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);

    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, logger);
    cargar_config(config);
    log_debug(logger, "Vamos a usar el algoritmo %s", kernel_config->ALGORITMO_PLANIFICACION);

	t_list* estados[5] = inicializar_estados();
    inicializar_diccionario_recursos();

    inicializar_semaforos();

    // Conexiones con los demas modulos
    conexionCPU = armar_conexion(config, CPU, logger);
    conexionMemoria = armar_conexion(config, MEMORIA, logger);
    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, logger);

    int servidor_kernel = iniciar_servidor(config, logger);

    inicializar_planificador();

    // TODO Manejar multiples instancias de conexiones de consola al kernel
    inicializar_escucha_conexiones_consolas(servidor_kernel);

    free(kernel_config);

    return 0;
}




void cargar_config(t_config* config){
	kernel_config = malloc(sizeof(t_kernel_config));
	//kernel_config->IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
	kernel_config->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	kernel_config->PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	kernel_config->IP_FILE_SYSTEM = config_get_string_value(config, "IP_FILE_SYSTEM");
	kernel_config->PUERTO_FILE_SYSTEM = config_get_string_value(config, "PUERTO_FILE_SYSTEM");
	kernel_config->IP_CPU = config_get_string_value(config, "IP_CPU");
	kernel_config->PUERTO_CPU = config_get_string_value(config, "PUERTO_CPU");
	kernel_config->PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	kernel_config->ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	kernel_config->ESTIMACION_INICIAL = config_get_string_value(config, "ESTIMACION_INICIAL");
	kernel_config->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
	kernel_config->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
	kernel_config->RECURSOS = config_get_array_value(config, "RECURSOS");
	kernel_config->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");

	log_info(logger, "Config cargada en  'kernel_config' ");
}

void iterator(char* value) {
    log_info(logger,"%s", value);
}

t_list* procesar_instrucciones(int clienteAceptado,t_list* lista_instrucciones, t_log* logger, t_config* config){
    int instruccion = recibir_operacion(clienteAceptado);
    lista_instrucciones = recibir_paquete(clienteAceptado);

    return lista_instrucciones;
}

void inicializar_escucha_conexiones_consolas(int servidor_kernel){

	log_info(logger, "Esperando conexiones de las consolas...\n");

	while(1){
		int socket_cliente = accept(servidor_kernel, NULL, NULL);
		log_info(logger, "Consola conectada!\n");
		pthread_t hilo_consola;
		pthread_create(&hilo_consola, NULL, (void *) recibir_de_consola, (void *) socket_cliente);
		pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
	}
}

void recibir_de_consola(int socket_cliente) {
	while(1){  // Queda en un estado de espera activa para la comunicación continua entre los módulos.
		int codigo_de_operacion;
		int cod_op;
		if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0){
		    codigo_de_operacion = cod_op;
		}else{
		    close(socket_cliente);
		    codigo_de_operacion = -1;
		}
		PCB* pcb;
		switch(codigo_de_operacion){
			case INICIALIZAR_PROCESO:
				pcb = inicializar_pcb(socket_cliente);
		}
	}
}

PCB* inicializar_pcb(int socket_cliente){  // chequear que se lee bien de consola

	void* buffer;
	int tamanio = 0;
	int desplazamiento = 0;
	buffer = recibir_buffer(&tamanio, socket_cliente);
	char** instrucciones = read_string_array(buffer, &desplazamiento);

	wait(&sem_pcb_create);

	PCB* pcb = pcb_create(instructions, socket, pid_counter, segments);

    pid_counter++;
	log_info(logger, "valor id: %d", pcb->process_id);

	signal(&sem_pcb_create);

	return pcb;
}







