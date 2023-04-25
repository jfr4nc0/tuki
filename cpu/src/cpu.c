#include "../include/cpu.h"
#include "utils.c"

int main(int argc, char** argv)
{
    int clienteAceptado;

    t_log* logger;
    t_config* config;

    logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);

    // Levanta archivo de configuración
    config = iniciar_config(DEFAULT_CONFIG_PATH, logger);

    // CONEXION CON MEMORIA - cliente
    int conexionMemoria = armar_conexion(config, MEMORIA, logger);

    // CONEXION CON KERNEL - servidor
    int servidorCPU = iniciar_servidor(config, logger);
    clienteAceptado = esperar_cliente(servidorCpu, logger);

    terminar_programa(servidorCpu, logger, config);
}
