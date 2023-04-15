#include "../include/cpu.h"
#include "utils.c"

int main(int argc, char** argv)
{

    // Se setean los parametros que se pasan, con poner valores por defecto
    char* pathConfig = DEFAULT_CONFIG_PATH;
    char* pathLog = DEFAULT_LOG_PATH;

    int conexion;

    t_log* logger;
    t_config* config;

    logger = iniciar_logger(pathLog, ENUM_CPU);

    // Creamos una conexión hacia el servidor
    conexion = armar_conexion(config, CPU, logger);

    // Armamos y enviamos el paquete
    paquete(conexion, logger);

    terminar_programa(conexion, logger, config);
}
