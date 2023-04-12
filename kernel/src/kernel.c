#include "./../include/kernel.h"

t_log* logger;

int main(void) {
	logger = log_create(LOG_PATH_KERNEL, MODULO_KERNEL, MOSTRAR_OCULTAR_MENSAJES_LOG_KERNEL, LOG_LEVEL_KERNEL);

	int server_fd = iniciar_servidor();
	log_info(logger, I__SERVER_READY, MODULO_CONSOLA, ENTER);

	int cliente_fd = esperar_cliente(server_fd);

	t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
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

void iterator(char* value) {
	log_info(logger,"%s", value);
}
