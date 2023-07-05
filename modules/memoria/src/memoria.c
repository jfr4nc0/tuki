#include "memoria.h"
#include "administrarMemoria.h"

t_memoria_config* memoriaConfig;

int main() {
    loggerMemoria = iniciar_logger(DEFAULT_LOG_PATH, ENUM_MEMORIA);
    t_config* configInicial = iniciar_config(DEFAULT_CONFIG_PATH, loggerMemoria);
    cargar_config_memoria(configInicial);

    int servidorMemoria = iniciar_servidor(configInicial, loggerMemoria);

    atender_conexiones(servidorMemoria);

    inicializar_memoria(memoriaConfig->TAM_MEMORIA, memoriaConfig->TAM_SEGMENTO_0);
    terminar_programa(servidorMemoria, loggerMemoria, configInicial);
    free(memoriaConfig);
    liberar_memoria();

	return 0;
}

void atender_conexiones(int servidorMemoria) {
    while (1){
		int clienteAceptado = esperar_cliente(servidorMemoria, loggerMemoria);
        // Recibo una primera operación para saber que módulo se conectó
        int modulo = recibir_operacion(clienteAceptado);
        // No se almacena ya que se ignora, pero necesito llamarlo para liberar el mensaje
        recibir_paquete(clienteAceptado);

        administrar_instrucciones(clienteAceptado, modulo);
    }
    return;
}

void administrar_instrucciones(int clienteAceptado, int modulo) {
        pthread_t thread_cpu;
        pthread_t thread_kernel;
        pthread_t thread_file_system;

        // TODO: ver la posibilidad de hacerlo generico pasandole un parametro mas a ejecutar_instrucciones que diga que módulo es
        switch(modulo) {
            case AUX_SOY_CPU:
                pthread_create(&thread_cpu, NULL, (void*) ejecutar_cpu_pedido, (void*) (intptr_t) clienteAceptado);
                pthread_join(thread_cpu, NULL);
            break;
            case AUX_SOY_KERNEL:
    		    pthread_create(&thread_kernel, NULL, (void*)ejecutar_kernel_pedido, (void*) (intptr_t) clienteAceptado);
    		    pthread_join(thread_kernel, NULL);
            break;
            case AUX_SOY_FILE_SYSTEM:
    		    pthread_create(&thread_file_system, NULL, (void*)ejecutar_file_system_pedido, (void*) (intptr_t) clienteAceptado);
    		    pthread_join(thread_file_system, NULL);
            break;
        }
}

void ejecutar_file_system_pedido(void *clienteAceptado) {
	int  conexionConFileSystem = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones(conexionConFileSystem, FILE_SYSTEM);
}

void ejecutar_cpu_pedido(void *clienteAceptado) {
	int  conexionConCPU = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones(conexionConCPU, CPU);
}


void ejecutar_kernel_pedido(void *clienteAceptado) {
	int  conexionConKernel = (int) (intptr_t)clienteAceptado;

    ejecutar_instrucciones(conexionConKernel, KERNEL);
}

void cargar_config_memoria(t_config* configInicial) {
    memoriaConfig = malloc(sizeof(t_memoria_config));
    memoriaConfig->PUERTO_ESCUCHA = extraer_int_de_config(configInicial, "PUERTO_ESCUCHA", loggerMemoria);
    memoriaConfig->TAM_MEMORIA = extraer_int_de_config(configInicial, "TAM_MEMORIA", loggerMemoria);
    memoriaConfig->TAM_SEGMENTO_0 = extraer_int_de_config(configInicial, "TAM_SEGMENTO_0", loggerMemoria);
    memoriaConfig->CANT_SEGMENTOS = extraer_int_de_config(configInicial, "CANT_SEGMENTOS", loggerMemoria);
    memoriaConfig->RETARDO_MEMORIA = extraer_int_de_config(configInicial, "RETARDO_MEMORIA", loggerMemoria);
    memoriaConfig->RETARDO_COMPACTACION = extraer_int_de_config(configInicial, "RETARDO_COMPACTACION", loggerMemoria);
    memoriaConfig->ALGORITMO_ASIGNACION = extraer_string_de_config(configInicial, "ALGORITMO_ASIGNACION", loggerMemoria);

    log_info(loggerMemoria, I__CONFIG_GENERIDO_CARGADO, MEMORIA);
}

void iterator(char* value) {
    log_info(loggerMemoria, "%s ", value);
}

void ejecutar_instrucciones(int cliente, char* modulo) {
	log_info(loggerMemoria, "Esperando instrucciones de: %s ", modulo);
    int codigoDeOperacion = recibir_operacion(cliente);
    log_info(loggerMemoria, I__RECIBO_INSTRUCCION, codigoDeOperacion, modulo);
    t_list* listaRecibida = recibir_paquete(cliente);
    list_iterate(listaRecibida, (void*) iterator);
    enviar_mensaje("Instrucción recibida", cliente, loggerMemoria);

    return;
}
