#include "../include/fileSystem.h"

#include "../../shared/src/funcionesCliente.c"
#include "utils.c"

int main(int argc, char** argv)
{
	char* pathConfig = DEFAULT_CONFIG_PATH;
	char* pathLog =  DEFAULT_LOG_PATH;

	int conexion;
	t_log* logger;
	t_config* config;

	logger = iniciar_logger(pathLog);
	config = iniciar_config(pathConfig);

	// Creamos una conexión hacia el servidor
	conexion = armar_conexion(config, logger);

	// Armamos y enviamos el paquete
	paquete(conexion, logger);

	terminar_programa(conexion, logger, config);
}
