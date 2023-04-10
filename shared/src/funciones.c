#include "../funciones.h"
#include "../constantes.h"

void paquete(int conexion)
{
	t_paquete* paquete;
	char* lineaPaquete;

	if(!(paquete = crear_paquete())) {
		printf(E__PAQUETE_CREATE);
	}

	// Leemos y esta vez agregamos las lineas al paquete
	printf("Los siguientes valores que ingreses se enviaran al servidor, ingrese enter para terminar de ingresar valores\n");

	while(1) {
		lineaPaquete = readline(SIGN_CONSOLA);
		if (strcmp(lineaPaquete, "") == 0) {
			break;
		}
		agregar_a_paquete(paquete, lineaPaquete, strlen(lineaPaquete)+1);
		free(lineaPaquete);
	}

	enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

int armar_conexion(t_config* config, t_log* logger)
{
	char* ip = config_get_string_value(config, "IP");
	char* puerto = config_get_string_value(config, "PUERTO");

	log_info(logger, "Estableciendo conexion con valores:", ENTER);
	log_info(logger, "ip %s, puerto %s\n", ip, puerto);

	return crear_conexion(ip, puerto);
}


t_log* iniciar_logger(char* pathLog)
{
	t_log *logger;
	if (( logger = log_create(pathLog, "logs", true, LOG_LEVEL_INFO)) == NULL ) {
		printf(E__LOGGER_CREATE, ENTER);
		exit(1);
	}
	return logger;
}

t_config* iniciar_config(char* pathConfig)
{
	t_config* nuevo_config;
	if ((nuevo_config = config_create(pathConfig)) == NULL) {
		printf(E__LOGGER_CREATE, ENTER);
		exit(1);
	}

	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	printf("Los siguientes valores que ingresen se guardaran en el log, ingrese un enter para terminar de ingresar valores\n");
	char* linea;

	while(1) {
		linea = readline(SIGN_CONSOLA);
		if (strcmp(linea, "") == 0) {
			break;
		}
		log_info(logger, linea, ENTER);
		free(linea);
	}
}

void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	if (logger != NULL) {
		log_destroy(logger);
	}

	if (config != NULL) {
		config_destroy(config);
	}

	liberar_conexion(conexion);
}
