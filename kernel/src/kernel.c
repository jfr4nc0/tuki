#include "../include/kernel.h"
#include "../../shared/src/funciones.c"

t_log* logger;

int main(int argc, char** argv) {
	char* pathConfig = argv[1] ? argv[1] : PATH_DEFAULT_CONEXION_KERNEL;
	logger = log_create(PATH_LOG_KERNEL, MODULO_KERNEL, MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL, LOG_LEVEL_KERNEL);
	t_config* config = iniciar_config(PATH_CONFIG_KERNEL, logger);
	t_config* configConexionKernel = iniciar_config(pathConfig, logger);

	int servidorKernel = iniciar_servidor(configConexionKernel, MODULO_KERNEL);
	log_info(logger, I__SERVER_READY, MODULO_KERNEL, ENTER);

	// int conexionMemoria = armar_conexion(configConexionKernel, MODULO_MEMORIA, logger);
	// int conexionCpu = armar_conexion(configConexionKernel, MODULO_CPU, logger);

	int cliente_aceptado = esperar_cliente(servidorKernel);

	return procesarInstrucciones(cliente_aceptado, logger);
}



void iterator(char* value) {
	log_info(logger,"%s", value);
}


int procesarInstrucciones(int cliente_aceptado, t_log* logger) {
	t_list* lista;
	while (1) {
		int instruccion = recibir_operacion(cliente_aceptado);
		switch (instruccion) {
		case MENSAJE:
			recibir_mensaje(cliente_aceptado);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_aceptado);
			log_info(logger, "Me llegaron los siguientes valores:", ENTER);
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_info(logger, I__DESCONEXION_CLIENTE);
			return EXIT_FAILURE;
		default:
			log_warning(logger,"Operacion desconocida.");
			break;
		}
	}
	return EXIT_SUCCESS;
}
