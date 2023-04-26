#include "../include/fileSystem.h"
#include "../../shared/src/funciones.c"

int main(int argc, char** argv)
{
    int clienteAceptado;

    t_log* logger;
    t_config* config;

    logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_FILE_SYSTEM);

    config = iniciar_config(DEFAULT_CONFIG_PATH, logger);

    // Conexion con memoria
    int conexionMemoria = armar_conexion(config, MEMORIA, logger);

    int servidorFileSystem = iniciar_servidor(config, logger);
    clienteAceptado = esperar_cliente(servidorFileSystem, logger);

    terminar_programa(servidorFileSystem, logger, config);
}
