#include "memoria.h"

int main(int argc, char** argv) {
    t_log* logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_MEMORIA);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);

    int servidorMemoria = iniciar_servidor(config, logger);
    esperar_cliente(servidorMemoria, logger);

    terminar_programa(servidorMemoria, logger, config);

	return 0;
}
