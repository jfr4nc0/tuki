#include "../include/fileSystem.h"
#include "../../shared/src/funciones.c"

int main(int argc, char** argv)
{
    char* pathConfig = DEFAULT_CONFIG_PATH;
    char* pathLog =  DEFAULT_LOG_PATH;

    int conexion;
    t_log* logger;
    t_config* config;

    logger = iniciar_logger(pathLog, ENUM_FILE_SYSTEM);
    config = iniciar_config(pathConfig, logger);

    // Creamos una conexión hacia el servidor
    conexion = armar_conexion(config, FILE_SYSTEM, logger);

    // Armamos y enviamos el paquete
    paquete(conexion, logger);

    terminar_programa(conexion, logger, config);
}
