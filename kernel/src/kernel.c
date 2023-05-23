#include "../include/kernel.h"

int main(int argc, char** argv) {
    logger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);

    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, logger);
    kernel_config = cargar_config_kernel(config, logger);

    log_debug(logger, "Vamos a usar el algoritmo %s", kernel_config.ALGORITMO_PLANIFICACION);

    inicializar_semaforos();

    t_list* lista_estados[CANTIDAD_ESTADOS];
    inicializar_listas_estados(lista_estados);
    inicializar_diccionario_recursos(kernel_config);

    // Conexiones con los demas modulos
    int conexionCPU = armar_conexion(config, CPU, logger);
    int conexionMemoria = armar_conexion(config, MEMORIA, logger);
    int conexionFileSystem = armar_conexion(config, FILE_SYSTEM, logger);

    int servidorKernel = iniciar_servidor(config, logger);

    inicializar_planificador();

    // TODO: Manejar multiples instancias de conexiones de consola al kernel
    inicializar_escucha_conexiones_consolas(servidorKernel);

    return 0;
}

t_kernel_config cargar_config_kernel(t_config* config, t_log* logger) {
    t_kernel_config kernel_config;

    kernel_config.IP_MEMORIA = extraer_de_config(config, "IP_MEMORIA", logger);
    kernel_config.PUERTO_MEMORIA = extraer_de_config(config, "PUERTO_MEMORIA", logger);
    kernel_config.IP_FILE_SYSTEM = extraer_de_config(config, "IP_FILE_SYSTEM", logger);
    kernel_config.PUERTO_FILE_SYSTEM = extraer_de_config(config, "PUERTO_FILE_SYSTEM", logger);
    kernel_config.IP_CPU = extraer_de_config(config, "IP_CPU", logger);
    kernel_config.PUERTO_CPU = extraer_de_config(config, "PUERTO_CPU", logger);
    kernel_config.PUERTO_ESCUCHA = extraer_de_config(config, "PUERTO_ESCUCHA", logger);
    kernel_config.ALGORITMO_PLANIFICACION = extraer_de_config(config, "ALGORITMO_PLANIFICACION", logger);
    kernel_config.ESTIMACION_INICIAL = extraer_de_config(config, "ESTIMACION_INICIAL", logger);
    kernel_config.HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
    kernel_config.GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
    kernel_config.RECURSOS = config_get_array_value(config, "RECURSOS");
    kernel_config.INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");

    log_info(logger, "Config cargada en  'kernel_config' ");

    return kernel_config;
}

t_list* procesar_instrucciones(int clienteAceptado,t_list* lista_instrucciones, t_log* logger, t_config* config){
    int instruccion = recibir_operacion(clienteAceptado);
    lista_instrucciones = recibir_paquete(clienteAceptado);

    return lista_instrucciones;
}

void inicializar_escucha_conexiones_consolas(int servidorKernel){

    log_info(logger, cantidad_strings_a_mostrar(2), "Esperando conexiones de las consolas...", ENTER);

    while(1){
        int clienteAceptado = esperar_cliente(servidorKernel, logger);
        log_info(logger, cantidad_strings_a_mostrar(2), "Consola conectada!", ENTER);
        pthread_t hilo_consola;
        pthread_create(&hilo_consola, NULL, (void *) recibir_de_consola, (void *) clienteAceptado);
        pthread_detach(hilo_consola);  //Los recursos asociados se liberan automáticamente al finalizar.
    }
}

void recibir_de_consola(int clienteAceptado) {
    while(1){  // Queda en un estado de espera activa para la comunicación continua entre los módulos.
        int codigoDeOperacion = recibir_operacion(clienteAceptado);
        PCB* pcb;
        switch(codigoDeOperacion){
            case PAQUETE:
                t_list* listaInstrucciones = recibir_paquete(clienteAceptado);
                log_info(logger, cantidad_strings_a_mostrar(2), "Me llegaron los siguientes valores:", ENTER);
                list_iterate(listaInstrucciones, (void*) iterator);
                pcb = inicializar_pcb(clienteAceptado, listaInstrucciones);
                list_destroy(listaInstrucciones);
                break;
        }
    }
}

PCB* inicializar_pcb(int clienteAceptado, t_list* listaInstrucciones){  // chequear que se lee bien de consola

    wait(&sem_creacion_pcb);

    PCB* pcb = new_pcb(clienteAceptado, *listaInstrucciones);

    
    log_info(logger, "valor id: %d", pcb->id_proceso);

    signal(&sem_creacion_pcb);

    return pcb;
}

void iterator(char* value) {
    log_info(logger,"%s", value);
}
