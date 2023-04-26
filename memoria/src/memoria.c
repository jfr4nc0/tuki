#include "../include/memoria.h"

#include "../../shared/src/funciones.c"
#include "utils.c"

int main(int argc, char** argv)
{
    t_log* logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_MEMORIA);

    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);

    int servidorMemoria = iniciar_servidor(config, logger);

    int clienteAceptado = esperar_cliente(servidorMemoria, logger);

    terminar_programa(clienteAceptado, logger, config);
}
