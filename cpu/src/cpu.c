#include "../include/cpu.h"
#include "utils.c"

int main(int argc, char** argv)
{
    t_log* logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_CPU);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);

    int servidorCpu = iniciar_servidor(config, logger);

    // Conexion con memoria
    int conexionMemoria = armar_conexion(config, MEMORIA, logger);

    int clienteAceptado = esperar_cliente(servidorCpu, logger);

    terminar_programa(servidorCpu, logger, config);
}
