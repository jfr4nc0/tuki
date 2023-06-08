#include "fileSystem.h"

t_log* logger;

int main(int argc, char** argv) {
    logger = iniciar_logger(DEFAULT_LOG_PATH, ENUM_FILE_SYSTEM);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, logger);

    int conexionMemoria = armar_conexion(config, MEMORIA, logger);
    // Notifico a memoria que soy el módulo file system
    identificarse(conexionMemoria, AUX_SOY_FILE_SYSTEM);

    int servidorFileSystem = iniciar_servidor(config, logger);

    atender_kernel(servidorFileSystem);

    terminar_programa(servidorFileSystem, logger, config);
    liberar_conexion(conexionMemoria);

}

void atender_kernel(int servidorFileSystem) {
    while (1) {
        int clienteAceptado = esperar_cliente(servidorFileSystem, logger);
        pthread_t thread_kernel;

        pthread_create(&thread_kernel, NULL, (void*)ejecutar_instrucciones_kernel, (void*) (intptr_t) clienteAceptado);
        pthread_join(thread_kernel, NULL);


        liberar_conexion(clienteAceptado);
    }
}

void iterator(char* value) {
    log_info(logger, "%s ", value);
}

void ejecutar_instrucciones_kernel(void* cliente) {
    int clienteKernel = (int) (intptr_t) cliente;
    int codigoDeOperacion = recibir_operacion(clienteKernel);
    log_info(logger, "Recibida instruccion con codigo de operacion: %d, valores: ", codigoDeOperacion);
    t_list* listaRecibida = recibir_paquete(clienteKernel);
    list_iterate(listaRecibida, (void*) iterator);

    enviar_mensaje("Instrucción recibida", clienteKernel, logger);

    return;
}
