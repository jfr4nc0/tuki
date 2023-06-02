#include "fileSystem.h"

int main(int argc, char** argv) {
    t_log* logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_FILE_SYSTEM);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);

    int servidorFileSystem = iniciar_servidor(config, logger);

    int conexionMemoria = armar_conexion(config, MEMORIA, logger);

    int clienteAceptado = esperar_cliente(servidorFileSystem, logger);

    terminar_programa(servidorFileSystem, logger, config);
    liberar_conexion(conexionMemoria);
    liberar_conexion(clienteAceptado);
}
