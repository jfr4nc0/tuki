#include "fileSystem.h"


int main(int argc, char** argv) {
    loggerFileSystem = iniciar_logger(DEFAULT_LOG_PATH, ENUM_FILE_SYSTEM);
    t_config* config = iniciar_config(DEFAULT_CONFIG_PATH, loggerFileSystem);
    cargar_config(config);

    t_config* configPropio = iniciar_config(CONFIG_PROPIO_PATH, loggerFileSystem);

    int conexionMemoria = armar_conexion(config, MEMORIA, loggerFileSystem);
    // Notifico a memoria que soy el módulo file system
    enviar_codigo_operacion(conexionMemoria, AUX_SOY_FILE_SYSTEM);

    int servidorFileSystem = iniciar_servidor(config, loggerFileSystem);

    atender_kernel(servidorFileSystem);

    terminar_programa(servidorFileSystem, loggerFileSystem, config);
    liberar_conexion(conexionMemoria);

}

void atender_kernel(int servidorFileSystem) {
    while (1) {
        int clienteAceptado = esperar_cliente(servidorFileSystem, loggerFileSystem);
        pthread_t thread_kernel;

        pthread_create(&thread_kernel, NULL, (void*)ejecutar_instrucciones_kernel, (void*) (intptr_t) clienteAceptado);
        pthread_join(thread_kernel, NULL);


        liberar_conexion(clienteAceptado);
    }
}

void iterator(char* value) {
    log_info(loggerFileSystem, "%s ", value);
}

void ejecutar_instrucciones_kernel(void* cliente) {
    int clienteKernel = (int) (intptr_t) cliente;
    int codigoDeOperacion = recibir_operacion(clienteKernel);
    log_info(loggerFileSystem, "Recibida instruccion con codigo de operacion: %d, valores: ", codigoDeOperacion);
    t_list* listaRecibida = recibir_paquete(clienteKernel);
    list_iterate(listaRecibida, (void*) iterator);

    enviar_mensaje("Instrucción recibida", clienteKernel, loggerFileSystem);

    return;
}
