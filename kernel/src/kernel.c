#include "../include/kernel.h"
#include "../../shared/src/funciones.c"

t_log* logger;

int main(int argc, char** argv) {
    logger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);

    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, logger);

    //log_warning(logger, "Vamos a usar el algoritmo %s", )

    int servidorKernel = iniciar_servidor(config, logger);

    // Conexiones con los demas modulos
    int conexionCpu = armar_conexion(config, CPU, logger);
    int conexionMemoria = armar_conexion(config, MEMORIA, logger);
    int conexionFileSystem = armar_conexion(config, FILE_SYSTEM, logger);

    inicializar_planificador();
/*
    while(1){
    	log_info(logger, "Esperando un cliente nuevo de la consola...");
        int client_socket = wait_client(kernel_socket, logger);
    	log_info(logger, "Entro una consola con este socket: %d", client_socket);
        pthread_t attend_console;
        pthread_create(&attend_console, NULL, (void*) receive_console, (void*) client_socket);
        pthread_detach(attend_console);
    // Espera cliente
    }
*/
    return 0;
}

void inicializar_planificador() {
    log_info(logger, "Inicializando hilos...");
    pthread_create(&planificador_corto_plazo, NULL, (void*) schedule_next_to_running, NULL);
    pthread_detach(planificador_corto_plazo);

    pthread_create(&thread_memory, NULL, (void*) manage_memory, NULL);
    pthread_detach(thread_memory);

    //pthread_create(&thread_dispatch, NULL, (void*) manage_dispatch, (void*) connection_cpu_dispatch);
    //pthread_detach(thread_dispatch);

}

void schedule_next_to_running(){
}
void manage_memory(){
}
void manage_dispatch(){
}






