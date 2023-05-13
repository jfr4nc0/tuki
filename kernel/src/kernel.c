#include "../include/kernel.h"
#include "../../shared/src/funciones.c"

int main(int argc, char** argv) {
    logger = iniciar_logger(PATH_LOG_KERNEL, ENUM_KERNEL);

    t_config* config = iniciar_config(PATH_CONFIG_KERNEL, logger);
    cargar_config(config);
    log_debug(logger, "Vamos a usar el algoritmo %s", kernel_config->ALGORITMO_PLANIFICACION);

    inicializar_listas_estados();
    inicializar_diccionario_recursos();

    inicializar_semaforos();

    // Conexiones con los demas modulos
    conexionCPU = armar_conexion(config, CPU, logger);
    conexionMemoria = armar_conexion(config, MEMORIA, logger);
    conexionFileSystem = armar_conexion(config, FILE_SYSTEM, logger);

    int servidorKernel = iniciar_servidor(config, logger);

    inicializar_planificador();

    // TODO Manejar multiples instancias de conexiones de consola al kernel
    inicializar_escucha_conexiones_consolas();

    free(kernel_config);

    return 0;
}

void cargar_config(t_config* config){
	kernel_config = malloc(sizeof(t_kernel_config));
	//kernel_config->IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
	kernel_config->IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	kernel_config->PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	kernel_config->IP_FILE_SYSTEM = config_get_string_value(config, "IP_FILE_SYSTEM");
	kernel_config->PUERTO_FILE_SYSTEM = config_get_string_value(config, "PUERTO_FILE_SYSTEM");
	kernel_config->IP_CPU = config_get_string_value(config, "IP_CPU");
	kernel_config->PUERTO_CPU = config_get_string_value(config, "PUERTO_CPU");
	kernel_config->PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
	kernel_config->ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	kernel_config->ESTIMACION_INICIAL = config_get_string_value(config, "ESTIMACION_INICIAL");
	kernel_config->HRRN_ALFA = config_get_double_value(config, "HRRN_ALFA");
	kernel_config->GRADO_MAX_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MAX_MULTIPROGRAMACION");
	kernel_config->RECURSOS = config_get_array_value(config, "RECURSOS");
	kernel_config->INSTANCIAS_RECURSOS = config_get_array_value(config, "INSTANCIAS_RECURSOS");

	log_info(logger, "Config cargada en  'kernel_config' ");
}

void iterator(char* value) {
    log_info(logger,"%s", value);
}

t_list* procesar_instrucciones(int clienteAceptado,t_list* lista_instrucciones, t_log* logger, t_config* config){
    int instruccion = recibir_operacion(clienteAceptado);
    lista_instrucciones = recibir_paquete(clienteAceptado);

    return lista_instrucciones;
}

void inicializar_escucha_conexiones_consolas(){
	//TODO
	return;
}







