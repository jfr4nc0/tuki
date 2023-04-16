#include "../include/kernel.h"
#include "../../shared/src/funciones.c"

t_log* logger;

int main(int argc, char** argv) {
    char* pathConfig = PATH_DEFAULT_CONEXION_KERNEL;
    logger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);
    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, logger);

    int servidorKernel = iniciar_servidor(config, logger);

    // int conexionMemoria = armar_conexion(configConexionKernel, MEMORIA, logger);
    // int conexionCpu = armar_conexion(configConexionKernel, CPU, logger);

    int clienteAceptado = esperar_cliente(servidorKernel, logger);

    return procesar_instrucciones(clienteAceptado, logger, config);
}



void iterator(char* value) {
    log_info(logger,"%s", value);
}


int procesar_instrucciones(int clienteAceptado, t_log* logger, t_config* config) {
    t_list* lista;
    while (1) {
        int instruccion = recibir_operacion(clienteAceptado);
        switch (instruccion) {
        case MENSAJE:
            recibir_mensaje(clienteAceptado);
            break;
        case PAQUETE:
            lista = recibir_paquete(clienteAceptado);
            log_info(logger, "Me llegaron los siguientes valores:", ENTER);
            list_iterate(lista, (void*) iterator);
            break;
        case -1:
            log_info(logger, I__DESCONEXION_CLIENTE);
            terminar_programa(clienteAceptado, logger, config);
            return EXIT_FAILURE;
        default:
            log_warning(logger,"Operacion desconocida.");
            break;
        }
    }
    terminar_programa(clienteAceptado, logger, config);
    return EXIT_SUCCESS;
}
